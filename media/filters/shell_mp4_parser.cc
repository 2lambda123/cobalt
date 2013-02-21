/*
 * Copyright 2012 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "media/filters/shell_mp4_parser.h"

#include "base/stringprintf.h"
#include "media/base/shell_buffer_factory.h"
#include "lb_platform.h"
#include <inttypes.h>

#if SHELL_MP4_PARSER_DUMP_ATOMS
extern const std::string *global_game_content_path;
#endif

namespace media {

// how many bytes to skip within an avc1 before the config atoms start?
static const int kSkipBytes_avc1 = 78;

// what's the smallest meaningful mp4 atom size?
static const int kAtomMinSize = 8;

// what's the smallest meaningful esds we can use?
static const int kMinSize_esds = 23;

// how much to download of an hdlr to get the trak type?
static const int kDesiredBytes_hdlr = 12;
static const uint32 kAudioSubtype_hdlr_soun = 0x736f756e;
static const uint32 kVideoSubtype_hdlr_vide = 0x76696465;

// how much to download of an mp4a to determine version number?
static const int kDesiredBytes_mp4a = 2;
// how big is the mp4a atom before optional extension atoms?
static const int kTotalSize_mp4a_v0 = 28;
static const int kTotalSize_mp4a_v1 = 44;
static const int kTotalSize_mp4a_v2 = 64;

// how many bytes should we download from an mdhd?
static const int kDesiredBytes_mdhd = 20;

// how many bytes should we download from an mvhd?
static const int kDesiredBytes_mvhd = 20;

// how many bytes to skip within an stsd before the config atoms start?
static const int kSkipBytes_stsd = 8;

// For now use the same size of table for each mp4 map atom. Later we can
// collect data on individual optimizations and make more granularly sized
// tables.
static const uint32 kMapTableEntryCacheEntries = 2048;

// static
scoped_refptr<ShellParser> ShellMP4Parser::Construct(
    scoped_refptr<ShellDataSourceReader> reader,
    const uint8 *construction_header,
    const PipelineStatusCB &status_cb) {
  // detect mp4 stream by looking for ftyp atom at top of file
  uint32 ftyp = LB::Platform::load_uint32_big_endian(construction_header + 4);
  if (ftyp != kAtomType_ftyp) {
    // not an mp4
    return NULL;
  }

  // first 4 bytes will be the size of the ftyp atom
  uint32 ftyp_atom_size =
      LB::Platform::load_uint32_big_endian(construction_header);
  if (ftyp_atom_size < kAtomMinSize) {
    status_cb.Run(DEMUXER_ERROR_COULD_NOT_PARSE);
    return NULL;
  }

  // construct and return new mp4 parser
  return scoped_refptr<ShellParser>(new ShellMP4Parser(reader, ftyp_atom_size));
}

ShellMP4Parser::ShellMP4Parser(scoped_refptr<ShellDataSourceReader> reader,
                               uint32 ftyp_atom_size)
    : ShellAVCParser(reader)
    , atom_offset_(ftyp_atom_size)  // start at next atom, skipping over ftyp
    , current_trak_is_video_(false)
    , current_trak_is_audio_(false)
    , current_trak_time_scale_(0)
    , video_time_scale_hz_(0)
    , audio_time_scale_hz_(0)
    , audio_map_(new ShellMP4Map(reader))
    , video_map_(new ShellMP4Map(reader))
    , audio_sample_(0)
    , video_sample_(0) {
}

ShellMP4Parser::~ShellMP4Parser() {
}

// For MP4 we traverse the file's atom structure attempting to find the audio
// and video configuration information and the locations in the file of the
// various stbl subatoms which detail the position of the audio and video
// NALUs in the file. As some of the stbl subatoms can be quite large we cache
// a fixed maximum quantity of each stbl subatom and update the cache only on
// miss.
bool ShellMP4Parser::ParseConfig() {
  while (!IsConfigComplete() ||
         !audio_map_->IsComplete() ||
         !video_map_->IsComplete()) {
    if (!ParseNextAtom()) {
      return false;
    }
  }
  return true;
}

scoped_refptr<ShellAU> ShellMP4Parser::GetNextAU(DemuxerStream::Type type) {
  uint32 size = 0;
  uint32 duration_ticks = 0;
  uint64 timestamp_ticks = 0;
  uint64 offset = 0;
  bool is_keyframe = false;
  bool is_eos = false;
  uint32 time_scale_hz = 0;
  if (type == DemuxerStream::AUDIO) {
    if (!audio_map_->GetSize(audio_sample_, size)                 ||
        !audio_map_->GetOffset(audio_sample_, offset)             ||
        !audio_map_->GetDuration(audio_sample_, duration_ticks)   ||
        !audio_map_->GetTimestamp(audio_sample_, timestamp_ticks)) {
      // determine if EOS or error
      if (audio_map_->IsEOS(audio_sample_)) {
        return ShellAU::CreateEndOfStreamAU(DemuxerStream::AUDIO);
      } else {
        return NULL;
      }
    }
    // all aac frames are random-access, so all are keyframes
    is_keyframe = true;
    time_scale_hz = audio_time_scale_hz_;
    audio_sample_++;
  } else if (type == DemuxerStream::VIDEO) {
    if (!video_map_->GetSize(video_sample_, size)                 ||
        !video_map_->GetOffset(video_sample_, offset)             ||
        !video_map_->GetDuration(video_sample_, duration_ticks)   ||
        !video_map_->GetTimestamp(video_sample_, timestamp_ticks) ||
        !video_map_->GetIsKeyframe(video_sample_, is_keyframe)) {
      if (video_map_->IsEOS(video_sample_)) {
        return ShellAU::CreateEndOfStreamAU(DemuxerStream::VIDEO);
      } else {
        return NULL;
      }
    }
    video_sample_++;
    // The individual NALUs within the mdat atom include a size
    // at the front of each describing the size of the NALU. This
    // breaks the conversion to AnnexB, so we omit the size from
    // the AU and skip past downloading it.
    offset += nal_header_size_;
    size -= nal_header_size_;
    time_scale_hz = video_time_scale_hz_;
  } else {
    NOTREACHED() << "unsupported stream type";
    return NULL;
  }

  size_t prepend_size = CalculatePrependSize(type, is_keyframe);
  DCHECK(time_scale_hz);

  scoped_refptr<ShellAU> au = new ShellAU(
      type,
      offset,
      size,
      prepend_size,
      is_keyframe,
      TicksToTime(timestamp_ticks, time_scale_hz),
      TicksToTime(duration_ticks, time_scale_hz));
  return au;
}

// parse the atom starting at atom_offset_, update appropriate internal state,
// return false on fatal error. General structure of an MP4 atom is:
// field             | type   | comment
// ------------------+--------+---------
// atom size         | uint32 | if 0 means "rest of file", if 1 means extended
// fourCC code       | ASCII  | four-byte ASCII code we treat as uint32
// extended size     | uint64 | optional size field, only here if atom size is 1
// <--- rest of atom body starts here
bool ShellMP4Parser::ParseNextAtom() {
  uint8 atom[kAtomDownload];
  int bytes_read = reader_->BlockingRead(atom_offset_,
                                         kAtomDownload,
                                         atom);
  if (bytes_read < kAtomDownload) {
    return false;
  }
  // first 4 bytes are size of atom uint32
  uint64 atom_size = (uint64)LB::Platform::load_uint32_big_endian(atom);
  // normally atom body starts just past fourCC code
  uint32 atom_body = kAtomMinSize;
  // if 1 we need to load the extended size which will be appended just past
  // the fourCC code
  if (atom_size == 1) {
    atom_size = LB::Platform::load_uint64_big_endian(atom + 8);
    // advance atom_body past the 8 bytes of size we just parsed
    atom_body += 8;
  } else if (atom_size == 0) {
    // calculate size of this atom from remainder of file
    if (reader_->FileSize() > atom_offset_) {
      atom_size = reader_->FileSize() - atom_offset_;
    }
  }
  // atom sizes also include the size of the start of the atom, so sanity-check
  // the size we just parsed against the number of bytes we needed to parse it
  if (atom_size < atom_body) {
    DLOG(WARNING) << base::StringPrintf("atom size: %"PRId64" less than min body size %d",
        atom_size, atom_body);
    return false;
  }

  // extract fourCC code as big-endian uint32
  uint32 four_cc = LB::Platform::load_uint32_big_endian(atom + 4);
  DLOG(INFO) << base::StringPrintf("four_cc: %c%c%c%c", atom[4], atom[5], atom[6], atom[7]);

#if SHELL_MP4_PARSER_DUMP_ATOMS
  DumpAtomToDisk(four_cc, atom_size, atom_offset_);
#endif

  // advance read pointer to atom body
  atom_offset_ += atom_body;
  // adjust size of body of atom from size of header
  uint64 atom_data_size = atom_size - atom_body;

  // now take appropriate action based on atom type
  switch (four_cc) {
    // avc1 atoms are contained within stsd atoms and carry their own
    // configuration baggage load, which we skip over and parse the atoms
    // within, normally an avcC atom.
    case kAtomType_avc1:
      atom_offset_ += kSkipBytes_avc1;
      break;

    // avcC atoms contain the AVCConfigRecord, our video configuration info
    case kAtomType_avcC:
      if (!DownloadAndParseAVCConfigRecord(atom_offset_,
                                           atom_data_size)) {
        return false;
      }
      atom_offset_ += atom_data_size;
      break;

    // esds atoms contain actually usable audio configuration info for AAC.
    case kAtomType_esds:
      return ParseMP4_esds(atom_data_size);

    // can tell us if mdia and mdhd atoms relate to audio or video metadata
    case kAtomType_hdlr:
      return ParseMP4_hdlr(atom_data_size, atom + atom_body);

    // provides a duration and a timescale unique to a given track
    case kAtomType_mdhd:
      return ParseMP4_mdhd(atom_data_size, atom + atom_body);

    // mp4a atoms contain audio configuration info, but we only want to know
    // which version it is so we can skip to the esds, which we must be present
    // when using AAC
    case kAtomType_mp4a:
      return ParseMP4_mp4a(atom_data_size, atom + atom_body);

    // movie header atom contains track duration and time unit scale, we trust
    // these data as the authoritative duration data for the mp4
    case kAtomType_mvhd:
      return ParseMP4_mvhd(atom_data_size, atom + atom_body);

    // stsd atoms may contain avc1 atoms, which themselves may contain avcC
    // atoms, which contain actually usable configuration information. skip to
    // subatom.
    case kAtomType_stsd:
      atom_offset_ += kSkipBytes_stsd;
      break;

    // We're very much interested in the contents of the trak container atom,
    // blow away state that we may have been keeping about any prior trak
    // atoms we've parsed.
    case kAtomType_trak:
      current_trak_is_video_ = false;
      current_trak_is_audio_ = false;
      break;

    // if one of the stbl subatoms add it to the appropriate audio or video map
    // and then advance past it.
    case kAtomType_co64:
    case kAtomType_ctts:
    case kAtomType_stco:
    case kAtomType_stts:
    case kAtomType_stsc:
    case kAtomType_stss:
    case kAtomType_stsz:
      if (current_trak_is_video_) {
        video_map_->SetAtom(four_cc,
                            atom_offset_,
                            atom_data_size,
                            kMapTableEntryCacheEntries,
                            atom + atom_body);
      } else if (current_trak_is_audio_) {
        audio_map_->SetAtom(four_cc,
                            atom_offset_,
                            atom_data_size,
                            kMapTableEntryCacheEntries,
                            atom + atom_body);
      }
      atom_offset_ += atom_data_size;
      break;

    // these are container atoms, so we dont want to advance past the header
    // as we are interested in their contents. Parsing them is trivial
    // as all they are is a size header and a fourCC type tag, which we've
    // already parsed and advanced past.
    case kAtomType_mdia:
    case kAtomType_minf:
    case kAtomType_moov:
    case kAtomType_stbl:
      // no-op
      break;

    // known atom types that we wish to just skip past the body without warning
    case kAtomType_dinf:
    case kAtomType_dref:
    case kAtomType_smhd:
    case kAtomType_tkhd:
    case kAtomType_vmhd:
      atom_offset_ += atom_data_size;
      break;

    // parse functions are assumed to advance read_position_ themselves,
    // as we are flattening a tree of atoms so that the atom_size we parsed
    // this time, if it's a container, may not have been entirely consumed
    // in this single call. However for unsupported atoms we just skip them
    // entirely, meaning we will skip their contents too.
    default:
      atom_offset_ += atom_data_size;
      DLOG(INFO) << base::StringPrintf(
          "skipping unsupported MP4 atom: %c%c%c%c",
          atom[4], atom[5], atom[6], atom[7]);
      break;
  }

  return true;
}

#if SHELL_MP4_PARSER_DUMP_ATOMS

void ShellMP4Parser::DumpAtomToDisk(uint32 four_cc,
                                    uint32 atom_size,
                                    uint64 atom_offset) {
  // download entire atom into buffer
  scoped_refptr<ShellScopedArray> scoped_buffer =
      ShellBufferFactory::Instance()->AllocateArray(atom_size);
  uint8* buffer = scoped_buffer->Get();
  int bytes_read = reader_->BlockingRead(atom_offset,
                                         atom_size,
                                         buffer);
  DCHECK_EQ(bytes_read, atom_size);
  // calculate file and table names
  std::string av_prefix_file;
  if (current_trak_is_video_) {
    av_prefix_file = "/mp4_video_atom_";
  } else if (current_trak_is_audio_) {
    av_prefix_file = "/mp4_audio_atom_";
  } else {
    av_prefix_file = "/mp4_atom_";
  }
  std::string atom_name = base::StringPrintf("%c%c%c%c",
      (char)(four_cc >> 24), (char)(four_cc >> 16),
      (char)(four_cc >> 8), (char)four_cc);
  // build path
  std::string path = base::StringPrintf("%s%s%s_%lld.txt",
    *global_game_content_path.c_str(), av_prefix_file.c_str()
    atom_name.c_str(), atom_offset);
  // get file for writing
  FILE* atom_file = fopen(path.c_str(), "w");
  DCHECK(atom_file);
  // 13 bytes per line matches 80-column rule with indenting :)
  for (int i = 0; i < atom_size; i += 13) {
    std::string atom_chars;
    // do whole lines at a time
    if (atom_size - i > 13) {
      atom_chars = base::StringPrintf(
        "  0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, "
        "0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,\n",
        buffer[i     ], buffer[i + 1 ], buffer[i + 2 ], buffer[i + 3 ],
        buffer[i + 4 ], buffer[i + 5 ], buffer[i + 6 ], buffer[i + 7 ],
        buffer[i + 8 ], buffer[i + 9 ], buffer[i + 10], buffer[i + 11],
        buffer[i + 12]);
    } else {
      atom_chars = "  ";  // start string with indentation
      // do last line one char at a time
      for (int j = i; j < atom_size - 1; j++) {
        atom_chars += base::StringPrintf("0x%02x, ", buffer[j]);
      }
      // final char gets no comma and a newline
      atom_chars += base::StringPrintf("0x%02x\n", buffer[atom_size - 1]);
    }
    // save line
    fwrite(atom_chars.c_str(), 1, atom_chars.length(), atom_file);
  }
  // close file
  fclose(atom_file);
}

#endif

bool ShellMP4Parser::ParseMP4_esds(uint64 atom_data_size) {
  // sanity check for minimum size
  if (atom_data_size < kMinSize_esds) {
    DLOG(WARNING) << base::StringPrintf("bad size %"PRId64" on esds.",
      atom_data_size);
    return false;
  }
  // we'll need to download entire esds, allocate buffer for it
  scoped_refptr<ShellScopedArray> esds_storage =
      ShellBufferFactory::Instance()->AllocateArray(atom_data_size);
  uint8* esds = NULL;
  if (!esds_storage || !(esds = esds_storage->Get())) {
    DLOG(WARNING) << base::StringPrintf(
        "unable to allocate esds temp array of %"PRId64" bytes",
        atom_data_size);
    return false;
  }
  // download esds
  int bytes_read = reader_->BlockingRead(atom_offset_, atom_data_size, esds);
  if (bytes_read < atom_data_size) {
    DLOG(WARNING) << "failed to download esds";
    return false;
  }
  // esds has variable layout thanks to optional inclusion of several
  // "extended descriptor type strings" which means we must work our way
  // through it. for details see:
  // http://xhelmboyx.tripod.com/formats/mp4-layout.txt
  // first four bytes are version number, following byte is es descriptor type
  uint32 offset = 5;
  // parse next 3 bytes to see if they are extended descriptor type strings
  uint32 edts = LB::Platform::load_uint32_big_endian(esds + offset) >> 8;
  // three bytes of 0x80 or 0xfe should be skipped
  if (edts == 0x00808080 || edts == 0x00fefefe) {
    offset += 3;
  }
  // skip descriptor type length, es ID, stream priority, decoder config
  // descriptor type tag
  offset += 5;
  if (offset + 3 > atom_data_size) {
    DLOG(WARNING) << "parsed incomplete esds";
    return false;
  }
  // parse next 3 bytes to see if they are extended descriptor type strings
  edts = LB::Platform::load_uint32_big_endian(esds + offset) >> 8;
  // three bytes of 0x80 or 0xfe should be skipped
  if (edts == 0x00808080 || edts == 0x00fefefe) {
    offset += 3;
  }
  // skip descriptor type length, object type ID, stream type, buffer size,
  // max and min bit rates, and decoder specific descriptor type tag
  offset += 15;
  if (offset + 3 > atom_data_size) {
    DLOG(WARNING) << "parsed incomplete esds";
    return false;
  }
  edts = LB::Platform::load_uint32_big_endian(esds + offset) >> 8;
  // three bytes of 0x80 or 0xfe should be skipped
  if (edts == 0x00808080 || edts == 0x00fefefe) {
    offset += 3;
  }
  // min size check should have made sure we didn't run out of buffer by now
  // but adding up to 9 bytes to length may have run us out
  if (offset + 3 > atom_data_size) {
    DLOG(WARNING) << "parsed incomplete esds";
    return false;
  }
  // this byte describes the length in bytes of the audio specific config.
  // we need at least 2 bytes
  uint8 config_length = esds[offset];
  offset++;
  if (config_length >= 2) {
    ParseAudioSpecificConfig(esds[offset], esds[offset + 1]);
  } else {
    DLOG(WARNING) << "esds audio specific config shorter than 2 bytes";
    return false;
  }
  atom_offset_ += atom_data_size;
  return true;
}

bool ShellMP4Parser::ParseMP4_hdlr(uint64 atom_data_size, uint8* hdlr) {
  // ensure we're downloading enough of the hdlr to parse
  DCHECK_LE(kDesiredBytes_hdlr + 16, kAtomDownload);
  // sanity-check for minimum size
  if (atom_data_size < kDesiredBytes_hdlr) {
    DLOG(WARNING) << base::StringPrintf("bad size %"PRId64" on hdlr",
        atom_data_size);
    return false;
  }
  // last 4 bytes of the 12 we need are an ascii code for the trak type, we
  // want 'vide' for video or 'soun' for audio. ignore the rest.
  uint32 hdlr_subtype = LB::Platform::load_uint32_big_endian(hdlr + 8);
  // update state flags
  current_trak_is_video_ = (hdlr_subtype == kVideoSubtype_hdlr_vide);
  current_trak_is_audio_ = (hdlr_subtype == kAudioSubtype_hdlr_soun);
  // save a time scale if pending
  if (current_trak_time_scale_ > 0 && current_trak_is_video_) {
    video_time_scale_hz_ = current_trak_time_scale_;
    current_trak_time_scale_ = 0;
  }
  if (current_trak_time_scale_ > 0 && current_trak_is_audio_) {
    audio_time_scale_hz_ = current_trak_time_scale_;
    current_trak_time_scale_ = 0;
  }
  // skip rest of atom
  atom_offset_ += atom_data_size;
  return true;
}

bool ShellMP4Parser::ParseMP4_mdhd(uint64 atom_data_size, uint8* mdhd) {
  DCHECK_LE(kDesiredBytes_mdhd + 16, kAtomDownload);
  if (atom_data_size < kDesiredBytes_mdhd) {
    DLOG(WARNING) << base::StringPrintf("bad size %"PRId64" on mdhd",
        atom_data_size);
    return false;
  }
  uint32 time_scale = LB::Platform::load_uint32_big_endian(mdhd + 12);
  // double-check track duration, it may be different from the movie duration
  uint32 track_duration_ticks =
      LB::Platform::load_uint32_big_endian(mdhd + 16);
  base::TimeDelta track_duration = TicksToTime(track_duration_ticks,
                                               time_scale);
  if (track_duration < duration_) {
    DLOG(WARNING) << base::StringPrintf(
        "mdhd has shorter duration: %"PRId64" ms than old value: %"PRId64" ms.",
        track_duration.InMilliseconds(), duration_.InMilliseconds());
    duration_ = track_duration;
  }
  if (current_trak_is_video_) {
    video_time_scale_hz_ = time_scale;
    current_trak_time_scale_ = 0;
  } else if (current_trak_is_audio_) {
    audio_time_scale_hz_ = time_scale;
    current_trak_time_scale_ = 0;
  } else {
    // it's possible we will encounter the mdhd before we encounter the hdlr,
    // in that event we save the time scale value until we know.
    current_trak_time_scale_ = time_scale;
  }
  atom_offset_ += atom_data_size;
  return true;
}

bool ShellMP4Parser::ParseMP4_mp4a(uint64 atom_data_size, uint8* mp4a) {
  DCHECK_LE(kDesiredBytes_mp4a + 16, kAtomDownload);
  // we only need the first two bytes of the header, which details the version
  // number of this atom, which tells us the size of the rest of the header,
  // telling us how much we should skip to get to the extension contents.
  if (atom_data_size < kDesiredBytes_mp4a) {
    DLOG(WARNING) << base::StringPrintf("bad size %"PRId64" on mp4a",
        atom_data_size);
    return false;
  }
  uint16 mp4a_version = LB::Platform::load_uint16_big_endian(mp4a);
  switch (mp4a_version) {
    case 0:
      atom_offset_ += kTotalSize_mp4a_v0;
      return true;

    case 1:
      atom_offset_ += kTotalSize_mp4a_v1;
      return true;

    case 2:
      atom_offset_ += kTotalSize_mp4a_v2;
      return true;

    default:
      // unknown mp4a atom version, parse failure
      DLOG(ERROR) << base::StringPrintf(
          "parsed bad mp4a version %d", mp4a_version);
      return false;
  }
}

// partial layout of mvhd header is:
// offset | name              | size in bytes
// -------+-------------------+---------------
// 0      | version           | 1 (ignored)
// 1      | flags             | 3 (ignored)
// 4      | creation time     | 4 (ignored)
// 8      | modification time | 4 (ignored)
// 12     | time scale        | 4
// 16     | duration:         | 4
//
bool ShellMP4Parser::ParseMP4_mvhd(uint64 atom_data_size, uint8* mvhd) {
  DCHECK_LE(kDesiredBytes_mvhd + 16, kAtomDownload);
  // it should be at least long enough for us to extract the parts we want
  if (atom_data_size < kDesiredBytes_mvhd) {
    DLOG(WARNING) << base::StringPrintf("bad size %"PRId64" on mvhd",
        atom_data_size);
    return false;
  }
  uint32 time_scale_hz = LB::Platform::load_uint32_big_endian(mvhd + 12);
  if (!time_scale_hz) {
    DLOG(WARNING) << "got 0 time scale for mvhd";
    return false;
  }
  // duration is in units of the time scale we just extracted
  uint64 duration_ticks = LB::Platform::load_uint32_big_endian(mvhd + 16);
  // calculate actual duration from that and the time scale
  duration_ = TicksToTime(duration_ticks, time_scale_hz);
  // advance read position
  atom_offset_ += atom_data_size;
  return true;
}

base::TimeDelta ShellMP4Parser::TicksToTime(uint64 ticks,
                                            uint32 time_scale_hz) {
  DCHECK(time_scale_hz);
  return base::TimeDelta::FromMicroseconds((ticks * 1000000ULL) /
      time_scale_hz);
}

}  // namespace media
