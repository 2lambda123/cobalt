// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

((x, y = eval('x')) => assertEquals(42, y))(42);
((x, {y = eval('x')}) => assertEquals(42, y))(42, {});
