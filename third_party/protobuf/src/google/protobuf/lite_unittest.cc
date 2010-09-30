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

#include <string>
#include <iostream>

#include <google/protobuf/test_util_lite.h>
#include <google/protobuf/stubs/common.h>

using namespace std;

int main(int argc, char* argv[]) {
  string data, packed_data;

  {
    protobuf_unittest::TestAllTypesLite message, message2, message3;
    google::protobuf::TestUtilLite::ExpectClear(message);
    google::protobuf::TestUtilLite::SetAllFields(&message);
    message2.CopyFrom(message);
    data = message.SerializeAsString();
    message3.ParseFromString(data);
    google::protobuf::TestUtilLite::ExpectAllFieldsSet(message);
    google::protobuf::TestUtilLite::ExpectAllFieldsSet(message2);
    google::protobuf::TestUtilLite::ExpectAllFieldsSet(message3);
    google::protobuf::TestUtilLite::ModifyRepeatedFields(&message);
    google::protobuf::TestUtilLite::ExpectRepeatedFieldsModified(message);
    message.Clear();
    google::protobuf::TestUtilLite::ExpectClear(message);
  }

  {
    protobuf_unittest::TestAllExtensionsLite message, message2, message3;
    google::protobuf::TestUtilLite::ExpectExtensionsClear(message);
    google::protobuf::TestUtilLite::SetAllExtensions(&message);
    message2.CopyFrom(message);
    string extensions_data = message.SerializeAsString();
    GOOGLE_CHECK(extensions_data == data);
    message3.ParseFromString(extensions_data);
    google::protobuf::TestUtilLite::ExpectAllExtensionsSet(message);
    google::protobuf::TestUtilLite::ExpectAllExtensionsSet(message2);
    google::protobuf::TestUtilLite::ExpectAllExtensionsSet(message3);
    google::protobuf::TestUtilLite::ModifyRepeatedExtensions(&message);
    google::protobuf::TestUtilLite::ExpectRepeatedExtensionsModified(message);
    message.Clear();
    google::protobuf::TestUtilLite::ExpectExtensionsClear(message);
  }

  {
    protobuf_unittest::TestPackedTypesLite message, message2, message3;
    google::protobuf::TestUtilLite::ExpectPackedClear(message);
    google::protobuf::TestUtilLite::SetPackedFields(&message);
    message2.CopyFrom(message);
    packed_data = message.SerializeAsString();
    message3.ParseFromString(packed_data);
    google::protobuf::TestUtilLite::ExpectPackedFieldsSet(message);
    google::protobuf::TestUtilLite::ExpectPackedFieldsSet(message2);
    google::protobuf::TestUtilLite::ExpectPackedFieldsSet(message3);
    google::protobuf::TestUtilLite::ModifyPackedFields(&message);
    google::protobuf::TestUtilLite::ExpectPackedFieldsModified(message);
    message.Clear();
    google::protobuf::TestUtilLite::ExpectPackedClear(message);
  }

  {
    protobuf_unittest::TestPackedExtensionsLite message, message2, message3;
    google::protobuf::TestUtilLite::ExpectPackedExtensionsClear(message);
    google::protobuf::TestUtilLite::SetPackedExtensions(&message);
    message2.CopyFrom(message);
    string packed_extensions_data = message.SerializeAsString();
    GOOGLE_CHECK(packed_extensions_data == packed_data);
    message3.ParseFromString(packed_extensions_data);
    google::protobuf::TestUtilLite::ExpectPackedExtensionsSet(message);
    google::protobuf::TestUtilLite::ExpectPackedExtensionsSet(message2);
    google::protobuf::TestUtilLite::ExpectPackedExtensionsSet(message3);
    google::protobuf::TestUtilLite::ModifyPackedExtensions(&message);
    google::protobuf::TestUtilLite::ExpectPackedExtensionsModified(message);
    message.Clear();
    google::protobuf::TestUtilLite::ExpectPackedExtensionsClear(message);
  }

  cout << "PASS" << endl;
  return 0;
}
