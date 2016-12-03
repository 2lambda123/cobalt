/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


/**
 *  File Name:          try-009.js
 *  ECMA Section:
 *  Description:        The try statement
 *
 *  This test has a try block within a while block.  Verify that an exception
 *  breaks out of the while.  I don't really know why this is an interesting
 *  test case but Mike Shaver had two of these so what the hey.
 *
 *  Author:             christine@netscape.com
 *  Date:               11 August 1998
 */
var SECTION = "try-009";
var VERSION = "ECMA_2";
var TITLE   = "The try statement: try in a while block";

startTest();
writeHeaderToLog( SECTION + " "+ TITLE);

var EXCEPTION_STRING = "Exception thrown: ";
var NO_EXCEPTION_STRING = "No exception thrown: ";


TryInWhile( new TryObject( "hello", ThrowException, true ) );
TryInWhile( new TryObject( "aloha", NoException, false ));

test();

function TryObject( value, throwFunction, result ) {
  this.value = value;
  this.thrower = throwFunction;
  this.result = result;
}
function ThrowException() {
  throw EXCEPTION_STRING + this.value;
}
function NoException() {
  return NO_EXCEPTION_STRING + this.value;
}
function TryInWhile( object ) {
  result = null;
  while ( true ) {
    try {
      object.thrower();
      result = NO_EXCEPTION_STRING + object.value;
      break;
    } catch ( e ) {
      result = e;
      break;
    }
  }

  new TestCase(
    SECTION,
    "( "+ object  +".thrower() )",
    (object.result
     ? EXCEPTION_STRING + object.value :
     NO_EXCEPTION_STRING + object.value),
    result );
}
