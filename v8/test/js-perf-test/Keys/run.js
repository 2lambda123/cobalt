// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

load('../base.js');
load('keys.js');

var success = true;

function PrintResult(name, result) {
  print(name + '-Keys(Score): ' + result);
}

function PrintError(name, error) {
  PrintResult(name, error);
  success = false;
}

BenchmarkSuite.config.doWarmup = undefined;
BenchmarkSuite.config.doDeterministic = undefined;

BenchmarkSuite.RunSuites({NotifyResult: PrintResult, NotifyError: PrintError});
