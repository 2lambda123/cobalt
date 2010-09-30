// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)

#ifndef GOOGLE_PROTOBUF_COMPILER_MOCK_CODE_GENERATOR_H__
#define GOOGLE_PROTOBUF_COMPILER_MOCK_CODE_GENERATOR_H__

#include <string>
#include <google/protobuf/compiler/code_generator.h>

namespace google {
namespace protobuf {
namespace compiler {

// A mock CodeGenerator, used by command_line_interface_unittest.  This is in
// its own file so that it can be used both directly and as a plugin.
//
// Generate() produces some output which can be checked by ExpectCalled().  The
// generator can run in a different process (e.g. a plugin).
//
// If the parameter is "insert=NAMES", the MockCodeGenerator will insert lines
// into the files generated by other MockCodeGenerators instead of creating
// its own file.  NAMES is a comma-separated list of the names of those other
// MockCodeGenerators.
//
// MockCodeGenerator will also modify its behavior slightly if the input file
// contains a message type with one of the following names:
//   MockCodeGenerator_Error:  Causes Generate() to return false and set the
//     error message to "Saw message type MockCodeGenerator_Error."
//   MockCodeGenerator_Exit:  Generate() prints "Saw message type
//     MockCodeGenerator_Exit." to stderr and then calls exit(123).
//   MockCodeGenerator_Abort:  Generate() prints "Saw message type
//     MockCodeGenerator_Abort." to stderr and then calls abort().
class MockCodeGenerator : public CodeGenerator {
 public:
  MockCodeGenerator(const string& name);
  virtual ~MockCodeGenerator();

  // Expect (via gTest) that a MockCodeGenerator with the given name was called
  // with the given parameters by inspecting the output location.
  //
  // |insertions| is a comma-separated list of names of MockCodeGenerators which
  // should have inserted lines into this file.
  static void ExpectGenerated(const string& name,
                              const string& parameter,
                              const string& insertions,
                              const string& file,
                              const string& first_message_name,
                              const string& output_directory);

  // Get the name of the file which would be written by the given generator.
  static string GetOutputFileName(const string& generator_name,
                                  const FileDescriptor* file);
  static string GetOutputFileName(const string& generator_name,
                                  const string& file);

  // implements CodeGenerator ----------------------------------------

  virtual bool Generate(const FileDescriptor* file,
                        const string& parameter,
                        OutputDirectory* output_directory,
                        string* error) const;

 private:
  string name_;

  static string GetOutputFileContent(const string& generator_name,
                                     const string& parameter,
                                     const FileDescriptor* file);
  static string GetOutputFileContent(const string& generator_name,
                                     const string& parameter,
                                     const string& file,
                                     const string& first_message_name);
};

}  // namespace compiler
}  // namespace protobuf

}  // namespace google
#endif  // GOOGLE_PROTOBUF_COMPILER_MOCK_CODE_GENERATOR_H__
