// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BASE_INTERLEAVED_SINC_RESAMPLER_H_
#define MEDIA_BASE_INTERLEAVED_SINC_RESAMPLER_H_

#include <queue>

#include "base/memory/aligned_memory.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "media/base/buffers.h"
#include "media/base/media_export.h"

namespace media {

// InterleavedSincResampler is a high-quality interleaved multi-channel sample
//-rate converter operating on samples in float. It uses the same algorithm as
// SincResampler. Unlike SincResampler, it works in push mode instead of pull
// mode.
class MEDIA_EXPORT InterleavedSincResampler {
 public:
  // |io_sample_rate_ratio| is the ratio of input / output sample rates.
  // |channel_count| is the number of channels in the interleaved audio stream.
  InterleavedSincResampler(double io_sample_rate_ratio, int channel_count);

  // Append a buffer to the queue. The samples in the buffer has to be floats.
  void QueueBuffer(const scoped_refptr<Buffer>& buffer);

  // Resample |frames| of data from enqueued buffers.  Return false if no sample
  // is read.  Return true if all requested samples have been written into
  // |destination|.  It will never do a partial read.  After the stream reaches
  // the end, the function will fill the rest of buffer with 0.
  bool Resample(float* destination, int frames);

  // Flush all buffered data and reset internal indices.
  void Flush();

  // Return false if we shouldn't queue more buffers to the resampler.
  bool CanQueueBuffer() const;

  // Returning true when we start to return zero filled data because of EOS.
  bool ReachedEOS() const;

 private:
  void InitializeKernel();
  bool HasEnoughData(int frames_to_resample) const;
  void Read(float* destination, int frames);

  float Convolve(const float* input_ptr, const float* k1, const float* k2,
                 double kernel_interpolation_factor);

  // The ratio of input / output sample rates.
  double io_sample_rate_ratio_;

  // An index on the source input buffer with sub-sample precision.  It must be
  // double precision to avoid drift.
  double virtual_source_idx_;

  // The buffer is primed once at the very beginning of processing.
  bool buffer_primed_;

  // Number of audio channels.
  int channel_count_;

  // The size of bytes for an audio frame.
  const int frame_size_in_bytes_;

  // Contains kKernelOffsetCount kernels back-to-back, each of size kKernelSize.
  // The kernel offsets are sub-sample shifts of a windowed sinc shifted from
  // 0.0 to 1.0 sample.
  scoped_ptr_malloc<float, base::ScopedPtrAlignedFree> kernel_storage_;

  // Data from the source is copied into this buffer for each processing pass.
  scoped_ptr_malloc<float, base::ScopedPtrAlignedFree> input_buffer_;

  // A queue of buffers to be resampled.
  std::queue<scoped_refptr<Buffer> > pending_buffers_;

  // The current offset to read when reading from the first pending buffer.
  int offset_in_frames_;

  // The following two variables are used to calculate EOS and in HasEnoughData.
  int frames_resampled_;
  int frames_queued_;

  // Pointers to the various regions inside |input_buffer_|.  See the diagram at
  // the top of the .cc file for more information.
  float* const r0_;
  float* const r1_;
  float* const r2_;
  float* const r3_;
  float* const r4_;
  float* const r5_;

  DISALLOW_COPY_AND_ASSIGN(InterleavedSincResampler);
};

}  // namespace media

#endif  // MEDIA_BASE_INTERLEAVED_SINC_RESAMPLER_H_
