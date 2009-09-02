// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_TOOLS_DUMP_CACHE_URL_TO_FILE_ENCODER_H_
#define NET_TOOLS_DUMP_CACHE_URL_TO_FILE_ENCODER_H_

#include "base/file_path.h"
#include "base/file_util.h"
#include "base/string_util.h"
#include "googleurl/src/gurl.h"

namespace net {

// Helper class for converting a URL into a filename.
class UrlToFilenameEncoder {
 public:
  // Given a |url| and a |base_path|, returns a FilePath which represents this
  // |url|.
  static FilePath Encode(const std::string& url, FilePath base_path) {
    std::string clean_url(url);
    if (clean_url.length() && clean_url[clean_url.length()-1] == '/')
      clean_url.append("index.html");

    GURL gurl(clean_url);
    FilePath filename(base_path);
    filename = filename.Append(ASCIIToWide(gurl.host()));

    std::wstring url_filename = ASCIIToWide(gurl.PathForRequest());
    // Strip the leading '/'
    if (url_filename[0] == L'/')
      url_filename = url_filename.substr(1);

    // replace '/' with '\'
    ConvertToSlashes(url_filename);

    // strip double slashes ("\\")
    StripDoubleSlashes(url_filename);

    // Save path as filesystem-safe characters
    url_filename = Escape(url_filename);
    filename = filename.Append(url_filename);

    return filename;
  }

 private:
  // This is the length at which we chop individual subdirectories.
  // Technically, we shouldn't need to do this, but I found that
  // even with long-filename support, windows had trouble creating
  // long subdirectories, and making them shorter helps.
  static const int kMaximumSubdirectoryLength = 128;

  // Escape the given input |path| and chop any individual components
  // of the path which are greater than kMaximumSubdirectoryLength characters
  // into two chunks.
  static std::wstring Escape(const std::wstring& path) {
    std::wstring output;
    int last_slash = 0;
    for (size_t index = 0; index < path.length(); index++) {
      wchar_t wide_char = path[index];
      DCHECK((wide_char & 0xff) == wide_char);
      char ch = static_cast<char>(wide_char & 0xff);
      if (ch == 0x5C)
        last_slash = index;
      if ((ch == 0x2D) ||                   // hyphen
          (ch == 0x5C) || (ch == 0x5F) ||   // backslash, underscore
          ((0x30 <= ch) && (ch <= 0x39)) || // Digits [0-9]
          ((0x41 <= ch) && (ch <= 0x5A)) || // Uppercase [A-Z]
          ((0x61 <= ch) && (ch <= 0x7A))) { // Lowercase [a-z]
        output.append(&path[index],1);
      } else {
        wchar_t encoded[3];
        encoded[0] = L'x';
        encoded[1] = ch / 16;
        encoded[1] += (encoded[1] >= 10) ? 'A' - 10 : '0';
        encoded[2] = ch % 16;
        encoded[2] += (encoded[2] >= 10) ? 'A' - 10 : '0';
        output.append(encoded, 3);
      }
      if (index - last_slash > kMaximumSubdirectoryLength) {
        wchar_t backslash = L'\\';
        output.append(&backslash, 1);
        last_slash = index;
      }
    }
    return output;
  }

  // Replace all instances of |from| within |str| as |to|.
  static void ReplaceAll(std::wstring& str, const std::wstring& from,
                  const std::wstring& to) {
    std::wstring::size_type pos(0);
    while((pos = str.find(from, pos)) != std::wstring::npos) {
      str.replace(pos, from.size(), to);
      pos += from.size();
    }
  }

  // Replace all instances of "/" with "\" in |path|.
  static void ConvertToSlashes(std::wstring& path) {
    std::wstring slash(L"/");
    std::wstring backslash(L"\\");
    ReplaceAll(path, slash, backslash);
  }

  // Replace all instances of "\\" with "%5C%5C" in |path|.
  static void StripDoubleSlashes(std::wstring& path) {
    std::wstring::size_type pos(0);
    std::wstring doubleslash(L"\\\\");
    std::wstring escaped_doubleslash(L"%5C%5C");
    ReplaceAll(path, doubleslash, escaped_doubleslash);
  }
};

} // namespace net

#endif  // NET_TOOLS_DUMP_CACHE_URL_TO_FILE_ENCODER_H__

