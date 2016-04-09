// Copyright 2015 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// The Starboard configuration for Desktop Linux. Other devices will have
// specific Starboard implementations, even if they ultimately are running some
// version of Linux -- but they may base their configuration on the Desktop
// Linux configuration headers.

// Other source files should never include this header directly, but should
// include the generic "starboard/configuration.h" instead.

#ifndef STARBOARD_LINUX_SHARED_CONFIGURATION_PUBLIC_H_
#define STARBOARD_LINUX_SHARED_CONFIGURATION_PUBLIC_H_

// The API version implemented by this platform.
#define SB_API_VERSION 1

// --- System Header Configuration -------------------------------------------

// Any system headers listed here that are not provided by the platform will be
// emulated in starboard/types.h.

// Whether the current platform provides the standard header stdarg.h.
#define SB_HAS_STDARG_H 1

// Whether the current platform provides the standard header stdbool.h.
#define SB_HAS_STDBOOL_H 1

// Whether the current platform provides the standard header stddef.h.
#define SB_HAS_STDDEF_H 1

// Whether the current platform provides the standard header stdint.h.
#define SB_HAS_STDINT_H 1

// Whether the current platform provides the standard header inttypes.h.
#define SB_HAS_INTTYPES_H 1

// Whether the current platform provides the standard header wchar.h.
#define SB_HAS_WCHAR_H 1

// Whether the current platform provides the standard header limits.h.
#define SB_HAS_LIMITS_H 1

// Type detection for wchar_t.
#if defined(__WCHAR_MAX__) && \
    (__WCHAR_MAX__ == 0x7fffffff || __WCHAR_MAX__ == 0xffffffff)
#define SB_IS_WCHAR_T_UTF32 1
#elif defined(__WCHAR_MAX__) && \
    (__WCHAR_MAX__ == 0x7fff || __WCHAR_MAX__ == 0xffff)
#define SB_IS_WCHAR_T_UTF16 1
#endif

// Chrome only defines these two if ARMEL or MIPSEL are defined.
#if defined(__ARMEL__)
// Chrome has an exclusion for iOS here, we should too when we support iOS.
#define SB_IS_WCHAR_T_UNSIGNED 1
#elif defined(__MIPSEL__)
#define SB_IS_WCHAR_T_SIGNED 1
#endif

// --- Attribute Configuration -----------------------------------------------

// The platform's annotation for forcing a C function to be inlined.
#define SB_C_FORCE_INLINE __inline__ __attribute__((always_inline))

// The platform's annotation for marking a C function as suggested to be
// inlined.
#define SB_C_INLINE inline

// The platform's annotation for marking a C function as forcibly not
// inlined.
#define SB_C_NOINLINE __attribute__((noinline))

// The platform's annotation for marking a symbol as exported outside of the
// current shared library.
#define SB_EXPORT_PLATFORM __attribute__((visibility("default")))

// The platform's annotation for marking a symbol as imported from outside of
// the current linking unit.
#define SB_IMPORT_PLATFORM

// --- Extensions Configuration ----------------------------------------------

// GCC/Clang doesn't define a long long hash function, except for Android and
// Game consoles.
#define SB_HAS_LONG_LONG_HASH 0

// GCC/Clang doesn't define a string hash function, except for Game Consoles.
#define SB_HAS_STRING_HASH 0

// Desktop Linux needs a using statement for the hash functions.
#define SB_HAS_HASH_USING 0

// Set this to 1 if hash functions for custom types can be defined as a
// hash_value() function. Otherwise, they need to be placed inside a
// partially-specified hash struct template with an operator().
#define SB_HAS_HASH_VALUE 0

// Set this to 1 if use of hash_map or hash_set causes a deprecation warning
// (which then breaks the build).
#define SB_HAS_HASH_WARNING 1

// The location to include hash_map on this platform.
#define SB_HASH_MAP_INCLUDE <ext/hash_map>

// C++'s hash_map and hash_set are often found in different namespaces depending
// on the compiler.
#define SB_HASH_NAMESPACE __gnu_cxx

// The location to include hash_set on this platform.
#define SB_HASH_SET_INCLUDE <ext/hash_set>

// Define this to how this platform copies varargs blocks.
#define SB_VA_COPY(dest, source) va_copy(dest, source)

// --- Filesystem Configuration ----------------------------------------------

// The current platform's maximum length of the name of a single directory
// entry, not including the absolute path.
#define SB_FILE_MAX_NAME 64

// The current platform's maximum length of an absolute path.
#define SB_FILE_MAX_PATH 4096

// The current platform's file path component separator character. This is the
// character that appears after a directory in a file path. For example, the
// absolute canonical path of the file "/path/to/a/file.txt" uses '/' as a path
// component separator character.
#define SB_FILE_SEP_CHAR '/'

// The current platform's alternate file path component separator character.
// This is like SB_FILE_SEP_CHAR, except if your platform supports an alternate
// character, then you can place that here. For example, on windows machines,
// the primary separator character is probably '\', but the alternate is '/'.
#define SB_FILE_ALT_SEP_CHAR '/'

// The current platform's search path component separator character. When
// specifying an ordered list of absolute paths of directories to search for a
// given reason, this is the character that appears between entries. For
// example, the search path of "/etc/search/first:/etc/search/second" uses ':'
// as a search path component separator character.
#define SB_PATH_SEP_CHAR ':'

// The string form of SB_FILE_SEP_CHAR.
#define SB_FILE_SEP_STRING "/"

// The string form of SB_FILE_ALT_SEP_CHAR.
#define SB_FILE_ALT_SEP_STRING "/"

// The string form of SB_PATH_SEP_CHAR.
#define SB_PATH_SEP_STRING ":"

// --- Memory Configuration --------------------------------------------------

// The memory page size, which controls the size of chunks on memory that
// allocators deal with, and the alignment of those chunks. This doesn't have to
// be the hardware-defined physical page size, but it should be a multiple of
// it.
#define SB_MEMORY_PAGE_SIZE 4096

// Whether this platform has and should use an MMAP function to map physical
// memory to the virtual address space.
#define SB_HAS_MMAP 1

// Whether this platform has and should use an growable heap (e.g. with sbrk())
// to map physical memory to the virtual address space.
#define SB_HAS_VIRTUAL_REGIONS 0

// Specifies the alignment for IO Buffers, in bytes. Some low-level network APIs
// may require buffers to have a specific alignment, and this is the place to
// specify that.
#define SB_NETWORK_IO_BUFFER_ALIGNMENT 16

// Determines the alignment that allocations should have on this platform.
#define SB_MALLOC_ALIGNMENT ((size_t)16U)

// Determines the threshhold of allocation size that should be done with mmap
// (if available), rather than allocated within the core heap.
#define SB_DEFAULT_MMAP_THRESHOLD ((size_t)(256 * 1024U))

// Defines the path where memory debugging logs should be written to.
#define SB_MEMORY_LOG_PATH "/tmp/starboard"

// --- Thread Configuration --------------------------------------------------

// Defines the maximum number of simultaneous threads for this platform. Some
// platforms require sharing thread handles with other kinds of system handles,
// like mutexes, so we want to keep this managable.
#define SB_MAX_THREADS 90

// --- Graphics Configuration ------------------------------------------------

// Specifies whether this platform has a performant OpenGL ES 2 implementation,
// which allows client applications to use GL rendering paths.
#define SB_HAS_GLES2 1

// Specifies whether this platform supports a performant accelerated blitter
// API. The basic requirement is a scaled, clipped, alpha-blended blit.
#define SB_HAS_BLITTER 0

// --- Media Configuration ---------------------------------------------------

// Specifies whether this platform has support for direct access to a decoder
// for at least H.264/AAC. The decoder is likely to be faster-than-realtime, and
// it is up to the client to retain decoded frames and display them synchronized
// with the audio. The decoder must be wired up to the DRM system such that it
// accepts encrypted samples.
#define SB_HAS_DECODER 1

// Specifies whether this platform has support for a decrypting elementary
// stream player for at least AES128-CTR/H.264/AAC. A player is responsible for
// ingesting an audio and video elementary stream, optionally-encrypted, and
// ultimately producing synchronized audio/video. If a player is defined, it
// must choose one of the supported composition methods below.
#define SB_HAS_PLAYER 0

// Specifies whether this platform's player will produce an OpenGL texture that
// the client must draw every frame with its graphics rendering. It may be that
// we get a texture handle, but cannot perform operations like GlReadPixels on
// it if it is DRM-protected.
#define SB_IS_PLAYER_PRODUCING_TEXTURE 0

// Specifies whether this platform's player is composited with a formal
// compositor, where the client must specify how video is to be composited into
// the graphicals scene.
#define SB_IS_PLAYER_COMPOSITED 0

// Specifies whether this platform's player uses a "punch-out" model, where
// video is rendered to the far background, and the graphics plane is
// automatically composited on top of the video by the platform. The client must
// punch an alpha hole out of the graphics plane for video to show through.  In
// this case, changing the video bounds must be tightly synchronized between the
// player and the graphics plane.
#define SB_IS_PLAYER_PUNCHED_OUT 0

// Specifies the maximum amount of memory to reserve for media source audio
// buffers.
// TODO(***REMOVED***): Add some text about how to tune this.
#define SB_MEDIA_SOURCE_BUFFER_STREAM_AUDIO_MEMORY_LIMIT (3U * 1024U * 1024U)

// Specifies the maximum amount of memory to reserve for media source video
// buffers.
// TODO(***REMOVED***): Add some text about how to tune this.
#define SB_MEDIA_SOURCE_BUFFER_STREAM_VIDEO_MEMORY_LIMIT (16U * 1024U * 1024U)

// Specifies how much main CPU memory to reserve up-front for media source
// buffers.
// TODO(***REMOVED***): Add some text about how to tune this.
#define SB_MEDIA_MAIN_BUFFER_BUDGET (128U * 1024U * 1024U)

// Specifies how much GPU memory to reserve up-front for media source buffers.
// TODO(***REMOVED***): Add some text about how to tune this.
#define SB_MEDIA_GPU_BUFFER_BUDGET 0U

// Specifies how media buffers must be aligned on this platform.
#define SB_MEDIA_BUFFER_ALIGNMENT 128U

// --- Decoder-only Params? ---

// Specifies how video frame buffers must be aligned on this platform.
#define SB_MEDIA_VIDEO_FRAME_ALIGNMENT 256U

// TODO(***REMOVED***): Add description, and some text about how to tune this.
#define SB_MEDIA_MAXIMUM_VIDEO_PREROLL_FRAMES 4

// TODO(***REMOVED***): Add description, and some text about how to tune this.
#define SB_MEDIA_MAXIMUM_VIDEO_FRAMES 12

// --- Tuneable Parameters ---------------------------------------------------

// Specifies the network receive buffer size.
#define SB_NETWORK_RECEIVE_BUFFER_SIZE (16 * 1024)

// --- Platform Specific Audits ----------------------------------------------

#if !defined(__GNUC__)
#error "Linux builds need a GCC-like compiler (for the moment)."
#endif

#endif  // STARBOARD_LINUX_SHARED_CONFIGURATION_PUBLIC_H_
