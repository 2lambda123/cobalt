// onNewPromise handlers on different Debugger instances are independent.

var g = newGlobal();
var dbg1 = new Debugger(g);
var log1;
function h1(promise) {
  log1 += 'n';
  assertEq(promise.seen, undefined);
  promise.seen = true;
}

var dbg2 = new Debugger(g);
var log2;
function h2(promise) {
  log2 += 'n';
  assertEq(promise.seen, undefined);
  promise.seen = true;
}

log1 = log2 = '';
g.makeFakePromise();
assertEq(log1, '');
assertEq(log2, '');

log1 = log2 = '';
dbg1.onNewPromise = h1;
g.makeFakePromise();
assertEq(log1, 'n');
assertEq(log2, '');

log1 = log2 = '';
dbg2.onNewPromise = h2;
g.makeFakePromise();
assertEq(log1, 'n');
assertEq(log2, 'n');

log1 = log2 = '';
dbg1.onNewPromise = undefined;
g.makeFakePromise();
assertEq(log1, '');
assertEq(log2, 'n');
