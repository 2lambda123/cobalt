// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/**
 * The result of an addition is determined using the rules of IEEE 754 double-precision arithmetics
 *
 * @path ch11/11.6/11.6.1/S11.6.1_A4_T7.js
 * @description The sum of two nonzero finite values of the same magnitude and opposite sign is +0
 */

//CHECK#1
if (-Number.MIN_VALUE + Number.MIN_VALUE !== +0) {  
  $ERROR('#1.1: -Number.MIN_VALUE + Number.MIN_VALUE === 0. Actual: ' + (-Number.MIN_VALUE + Number.MIN_VALUE));
} else {
  if (1 / (-Number.MIN_VALUE + Number.MIN_VALUE) !== Number.POSITIVE_INFINITY) {
    $ERROR('#1.2: -Number.MIN_VALUE + Number.MIN_VALUE === + 0. Actual: -0');
  }
}

//CHECK#2
if (-Number.MAX_VALUE + Number.MAX_VALUE !== +0) {  
  $ERROR('#2.1: -Number.MAX_VALUE + Number.MAX_VALUE === 0. Actual: ' + (-Number.MAX_VALUE + Number.MAX_VALUE));
} else {
  if (1 / (-Number.MAX_VALUE + Number.MAX_VALUE) !== Number.POSITIVE_INFINITY) {
    $ERROR('#2.2: -Number.MAX_VALUE + Number.MAX_VALUE === + 0. Actual: -0');
  }
}

//CHECK#3
if (-1 / Number.MAX_VALUE + 1 / Number.MAX_VALUE !== +0) {  
  $ERROR('#3.1: -1 / Number.MAX_VALUE + 1 / Number.MAX_VALUE === 0. Actual: ' + (-1 / Number.MAX_VALUE + 1 / Number.MAX_VALUE));
} else {
  if (1 / (-1 / Number.MAX_VALUE + 1 / Number.MAX_VALUE) !== Number.POSITIVE_INFINITY) {
    $ERROR('#3.2: -1 / Number.MAX_VALUE + 1 / Number.MAX_VALUE === + 0. Actual: -0');
  }
}

