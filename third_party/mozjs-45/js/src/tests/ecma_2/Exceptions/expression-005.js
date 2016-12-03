/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


/**
   File Name:          expression-005.js
   Corresponds To:     11.2.2-10-n.js
   ECMA Section:       11.2.2. The new operator
   Description:

   Author:             christine@netscape.com
   Date:               12 november 1997
*/

var SECTION = "expression-005";
var VERSION = "JS1_4";
var TITLE   = "The new operator";

startTest();
writeHeaderToLog( SECTION + " "+ TITLE);

var result = "Failed";
var expect = "Passed";
var exception = "No exception thrown";

try {
  result = new Math();
} catch ( e ) {
  result = expect;
  exception = e.toString();
}

new TestCase(
  SECTION,
  "result= new Math() (threw " + exception + ")",
  expect,
  result );

test();
