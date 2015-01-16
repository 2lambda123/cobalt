/*
 * Copyright 2014 Google Inc. All Rights Reserved.
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

#ifndef CSS_PARSER_REF_COUNTED_UTIL_H_
#define CSS_PARSER_REF_COUNTED_UTIL_H_

#include "base/logging.h"
#include "base/memory/ref_counted.h"

namespace cobalt {
namespace css_parser {

// Handy way to add a reference to a newly created object:
//
//     A* a = AddRef(new A());
//
// NULL pointers are allowed and are simply passed through.
//
// Use of manually managed reference-counted objects should be limited
// to the code that interfaces with C-style APIs, for example Bison's
// YYSTYPE union.
template <typename T>
inline T* AddRef(T* ptr) {
  if (ptr) {
    ptr->AddRef();
  }
  return ptr;
}

// Takes ownership over reference-counted object that has exactly 1 reference.
// NULL pointers are allowed.
template <typename T>
inline scoped_refptr<T> MakeScopedRefPtrAndRelease(T* ptr) {
  DCHECK(ptr == NULL || ptr->HasOneRef());

  scoped_refptr<T> refptr(ptr);
  if (ptr) {
    ptr->Release();
  }
  return refptr;
}

}  // namespace css_parser
}  // namespace cobalt

#endif  // CSS_PARSER_REF_COUNTED_UTIL_H_
