// Copyright (c) 2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// FilePath is a container for pathnames stored in a platform's native string
// type, providing containers for manipulation in according with the
// platform's conventions for pathnames.  It supports the following path
// types:
//
//                   POSIX            Windows
//                   ---------------  ----------------------------------
// Fundamental type  char[]           wchar_t[]
// Encoding          unspecified*     UTF-16
// Separator         /                \, tolerant of /
// Drive letters     no               case-insensitive A-Z followed by :
// Alternate root    // (surprise!)   \\, for UNC paths
//
// * The encoding need not be specified on POSIX systems, although some
//   POSIX-compliant systems do specify an encoding.  Mac OS X uses UTF-8.
//   Linux does not specify an encoding, but in practice, the locale's
//   character set may be used.
//
// For more arcane bits of path trivia, see below.
//
// FilePath objects are intended to be used anywhere paths are.  An
// application may pass FilePath objects around internally, masking the
// underlying differences between systems, only differing in implementation
// where interfacing directly with the system.  For example, a single
// OpenFile(const FilePath &) function may be made available, allowing all
// callers to operate without regard to the underlying implementation.  On
// POSIX-like platforms, OpenFile might wrap fopen, and on Windows, it might
// wrap _wfopen_s, perhaps both by calling file_path.value().c_str().  This
// allows each platform to pass pathnames around without requiring conversions
// between encodings, which has an impact on performance, but more imporantly,
// has an impact on correctness on platforms that do not have well-defined
// encodings for pathnames.
//
// Several methods are available to perform common operations on a FilePath
// object, such as determining the parent directory (DirName), isolating the
// final path component (BaseName), and appending a relative pathname string
// to an existing FilePath object (Append).  These methods are highly
// recommended over attempting to split and concatenate strings directly.
// These methods are based purely on string manipulation and knowledge of
// platform-specific pathname conventions, and do not consult the filesystem
// at all, making them safe to use without fear of blocking on I/O operations.
// These methods do not function as mutators but instead return distinct
// instances of FilePath objects, and are therefore safe to use on const
// objects.  The objects themselves are safe to share between threads.
//
// To aid in initialization of FilePath objects from string literals, a
// FILE_PATH_LITERAL macro is provided, which accounts for the difference
// between char[]-based pathnames on POSIX systems and wchar_t[]-based
// pathnames on Windows.
//
// Because a FilePath object should not be instantiated at the global scope,
// instead, use a FilePath::CharType[] and initialize it with
// FILE_PATH_LITERAL.  At runtime, a FilePath object can be created from the
// character array.  Example:
//
// | const FilePath::CharType kLogFileName[] = FILE_PATH_LITERAL("log.txt");
// |
// | void Function() {
// |   FilePath log_file_path(kLogFileName);
// |   [...]
// | }
//
// WARNING: FilePaths should ALWAYS be displayed with LTR directionality, even
// when the UI language is RTL. This means you always need to pass filepaths
// through l10n_util::WrapPathWithLTRFormatting() before displaying it in the
// RTL UI.
//
// This is a very common source of bugs, please try to keep this in mind.
//
// ARCANE BITS OF PATH TRIVIA
//
//  - A double leading slash is actually part of the POSIX standard.  Systems
//    are allowed to treat // as an alternate root, as Windows does for UNC
//    (network share) paths.  Most POSIX systems don't do anything special
//    with two leading slashes, but FilePath handles this case properly
//    in case it ever comes across such a system.  FilePath needs this support
//    for Windows UNC paths, anyway.
//    References:
//    The Open Group Base Specifications Issue 7, sections 3.266 ("Pathname")
//    and 4.12 ("Pathname Resolution"), available at:
//    http://www.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html#tag_03_266
//    http://www.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_12
//
//  - Windows treats c:\\ the same way it treats \\.  This was intended to
//    allow older applications that require drive letters to support UNC paths
//    like \\server\share\path, by permitting c:\\server\share\path as an
//    equivalent.  Since the OS treats these paths specially, FilePath needs
//    to do the same.  Since Windows can use either / or \ as the separator,
//    FilePath treats c://, c:\\, //, and \\ all equivalently.
//    Reference:
//    The Old New Thing, "Why is a drive letter permitted in front of UNC
//    paths (sometimes)?", available at:
//    http://blogs.msdn.com/oldnewthing/archive/2005/11/22/495740.aspx

#ifndef BASE_FILE_PATH_H_
#define BASE_FILE_PATH_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/hash_tables.h"
#include "base/string_piece.h"  // For implicit conversions.

// Windows-style drive letter support and pathname separator characters can be
// enabled and disabled independently, to aid testing.  These #defines are
// here so that the same setting can be used in both the implementation and
// in the unit test.
#if defined(OS_WIN)
#define FILE_PATH_USES_DRIVE_LETTERS
#define FILE_PATH_USES_WIN_SEPARATORS
#endif  // OS_WIN

// An abstraction to isolate users from the differences between native
// pathnames on different platforms.
class FilePath {
 public:
#if defined(OS_POSIX)
  // On most platforms, native pathnames are char arrays, and the encoding
  // may or may not be specified.  On Mac OS X, native pathnames are encoded
  // in UTF-8.
  typedef std::string StringType;
#elif defined(OS_WIN)
  // On Windows, for Unicode-aware applications, native pathnames are wchar_t
  // arrays encoded in UTF-16.
  typedef std::wstring StringType;
#endif  // OS_WIN

  typedef StringType::value_type CharType;

  // Null-terminated array of separators used to separate components in
  // hierarchical paths.  Each character in this array is a valid separator,
  // but kSeparators[0] is treated as the canonical separator and will be used
  // when composing pathnames.
  static const CharType kSeparators[];

  // A special path component meaning "this directory."
  static const CharType kCurrentDirectory[];

  // A special path component meaning "the parent directory."
  static const CharType kParentDirectory[];

  // The character used to identify a file extension.
  static const CharType kExtensionSeparator;

  FilePath() {}
  FilePath(const FilePath& that) : path_(that.path_) {}
  explicit FilePath(const StringType& path) : path_(path) {}

  FilePath& operator=(const FilePath& that) {
    path_ = that.path_;
    return *this;
  }

  bool operator==(const FilePath& that) const;

  bool operator!=(const FilePath& that) const;

  // Required for some STL containers and operations
  bool operator<(const FilePath& that) const {
    return path_ < that.path_;
  }

  const StringType& value() const { return path_; }

  bool empty() const { return path_.empty(); }

  // Returns true if |character| is in kSeparators.
  static bool IsSeparator(CharType character);

  // Returns a vector of all of the components of the provided path. It is
  // equivalent to calling DirName().value() on the path's root component,
  // and BaseName().value() on each child component.
  void GetComponents(std::vector<FilePath::StringType>* components) const;

  // Returns true, and sets *path to resulting full path, if relative_path can
  // be applied to current path by resolving any '..' it may contain. Returns
  // false otherwise, e.g., if relative path is absolute, or if it climbs back
  // up the hierarchy too far (i.e., beyond the root of current path).
  //
  // Note that if the current path ends with a file name, we won't try to
  // figure it out (so this method doesn't go to the disk) and we will blindly
  // append relative_path at the end of the current path, including the file
  // name in the current path (if any).
  bool AppendAndResolveRelative(const FilePath& relative_path,
                                FilePath* path) const;

  // Returns true if this FilePath is a strict parent of the |child|. Absolute
  // and relative paths are accepted i.e. is /foo parent to /foo/bar and
  // is foo parent to foo/bar. Does not convert paths to absolute, follow
  // symlinks or directory navigation (e.g. ".."). A path is *NOT* its own
  // parent.
  bool IsParent(const FilePath& child) const;

  // If IsParent(child) holds, appends to path (if non-NULL) the
  // relative path to child and returns true.  For example, if parent
  // holds "/Users/johndoe/Library/Application Support", child holds
  // "/Users/johndoe/Library/Application Support/Google/Chrome/Default", and
  // *path holds "/Users/johndoe/Library/Caches", then after
  // parent.AppendRelativePath(child, path) is called *path will hold
  // "/Users/johndoe/Library/Caches/Google/Chrome/Default".  Otherwise,
  // returns false.
  bool AppendRelativePath(const FilePath& child, FilePath* path) const;

  // Returns a FilePath corresponding to the directory containing the path
  // named by this object, stripping away the file component.  If this object
  // only contains one component, returns a FilePath identifying
  // kCurrentDirectory.  If this object already refers to the root directory,
  // returns a FilePath identifying the root directory.
  FilePath DirName() const;

  // Returns a FilePath corresponding to the last path component of this
  // object, either a file or a directory.  If this object already refers to
  // the root directory, returns a FilePath identifying the root directory;
  // this is the only situation in which BaseName will return an absolute path.
  FilePath BaseName() const;

  // Returns ".jpg" for path "C:\pics\jojo.jpg", or an empty string if
  // the file has no extension.  If non-empty, Extension() will always start
  // with precisely one ".".  The following code should always work regardless
  // of the value of path.
  // new_path = path.RemoveExtension().value().append(path.Extension());
  // ASSERT(new_path == path.value());
  // NOTE: this is different from the original file_util implementation which
  // returned the extension without a leading "." ("jpg" instead of ".jpg")
  StringType Extension() const;

  // Returns "C:\pics\jojo" for path "C:\pics\jojo.jpg"
  // NOTE: this is slightly different from the similar file_util implementation
  // which returned simply 'jojo'.
  FilePath RemoveExtension() const;

  // Inserts |suffix| after the file name portion of |path| but before the
  // extension.  Returns "" if BaseName() == "." or "..".
  // Examples:
  // path == "C:\pics\jojo.jpg" suffix == " (1)", returns "C:\pics\jojo (1).jpg"
  // path == "jojo.jpg"         suffix == " (1)", returns "jojo (1).jpg"
  // path == "C:\pics\jojo"     suffix == " (1)", returns "C:\pics\jojo (1)"
  // path == "C:\pics.old\jojo" suffix == " (1)", returns "C:\pics.old\jojo (1)"
  FilePath InsertBeforeExtension(const StringType& suffix) const;
  FilePath InsertBeforeExtensionASCII(const base::StringPiece& suffix) const;

  // Replaces the extension of |file_name| with |extension|.  If |file_name|
  // does not have an extension, them |extension| is added.  If |extension| is
  // empty, then the extension is removed from |file_name|.
  // Returns "" if BaseName() == "." or "..".
  FilePath ReplaceExtension(const StringType& extension) const;

  // Returns true if the file path matches the specified extension. The test is
  // case insensitive. Don't forget the leading period if appropriate.
  bool MatchesExtension(const StringType& extension) const;

  // Returns a FilePath by appending a separator and the supplied path
  // component to this object's path.  Append takes care to avoid adding
  // excessive separators if this object's path already ends with a separator.
  // If this object's path is kCurrentDirectory, a new FilePath corresponding
  // only to |component| is returned.  |component| must be a relative path;
  // it is an error to pass an absolute path.
  FilePath Append(const StringType& component) const WARN_UNUSED_RESULT;
  FilePath Append(const FilePath& component) const WARN_UNUSED_RESULT;

  // Although Windows StringType is std::wstring, since the encoding it uses for
  // paths is well defined, it can handle ASCII path components as well.
  // Mac uses UTF8, and since ASCII is a subset of that, it works there as well.
  // On Linux, although it can use any 8-bit encoding for paths, we assume that
  // ASCII is a valid subset, regardless of the encoding, since many operating
  // system paths will always be ASCII.
  FilePath AppendASCII(const base::StringPiece& component)
      const WARN_UNUSED_RESULT;

  // Returns true if this FilePath contains an absolute path.  On Windows, an
  // absolute path begins with either a drive letter specification followed by
  // a separator character, or with two separator characters.  On POSIX
  // platforms, an absolute path begins with a separator character.
  bool IsAbsolute() const;

  // Returns a copy of this FilePath that does not end with a trailing
  // separator.
  FilePath StripTrailingSeparators() const;

  // Returns true if this FilePath contains any attempt to reference a parent
  // directory (i.e. has a path component that is ".."
  bool ReferencesParent() const;

  // Older Chromium code assumes that paths are always wstrings.
  // This function converts a wstring to a FilePath, and is useful to smooth
  // porting that old code to the FilePath API.
  // It has "Hack" in its name so people feel bad about using it.
  // TODO(port): remove these functions.
  static FilePath FromWStringHack(const std::wstring& wstring);

  // Older Chromium code assumes that paths are always wstrings.
  // This function produces a wstring from a FilePath, and is useful to smooth
  // porting that old code to the FilePath API.
  // It has "Hack" in its name so people feel bad about using it.
  // TODO(port): remove these functions.
  std::wstring ToWStringHack() const;

 private:
  // Remove trailing separators from this object.  If the path is absolute, it
  // will never be stripped any more than to refer to the absolute root
  // directory, so "////" will become "/", not "".  A leading pair of
  // separators is never stripped, to support alternate roots.  This is used to
  // support UNC paths on Windows.
  void StripTrailingSeparatorsInternal();

  StringType path_;
};

// Macros for string literal initialization of FilePath::CharType[].
#if defined(OS_POSIX)
#define FILE_PATH_LITERAL(x) x
#elif defined(OS_WIN)
#define FILE_PATH_LITERAL(x) L ## x
#endif  // OS_WIN

// Provide a hash function so that hash_sets and maps can contain FilePath
// objects.
#if defined(COMPILER_GCC)
namespace __gnu_cxx {

template<>
struct hash<FilePath> {
  std::size_t operator()(const FilePath& f) const {
    return hash<FilePath::StringType>()(f.value());
  }
};

}  // namespace __gnu_cxx
#elif defined(COMPILER_MSVC)
namespace stdext {

inline size_t hash_value(const FilePath& f) {
  return hash_value(f.value());
}

}  // namespace stdext
#endif  // COMPILER

#endif  // BASE_FILE_PATH_H_
