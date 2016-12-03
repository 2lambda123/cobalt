load(libdir + "asserts.js");

var g = newGlobal();
for (var cls of [Map, Set]) {
    var getter = Object.getOwnPropertyDescriptor(cls.prototype, "size").get;
    assertThrowsInstanceOf(function () { getter.apply(g, []); }, g.TypeError);
}
