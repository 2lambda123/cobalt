// The Great Computer Language Shootout
// http://shootout.alioth.debian.org/
//
// contributed by Ian Osgood

function A(i,j) {
  return 1/((i+j)*(i+j+1)/2+i+1);
}

function Au(u,v) {
  for (var i=0; i<u.length; ++i) {
    var t = 0;
    for (var j=0; j<u.length; ++j)
      t += A(i,j) * u[j];
    v[i] = t;
  }
}

function Atu(u,v) {
  for (var i=0; i<u.length; ++i) {
    var t = 0;
    for (var j=0; j<u.length; ++j)
      t += A(j,i) * u[j];
    v[i] = t;
  }
}

function AtAu(u,v,w) {
  Au(u,w);
  Atu(w,v);
}

function spectralnorm(n) {
  var i, u=[], v=[], w=[], vv=0, vBv=0;
  for (i=0; i<n; ++i) {
    u[i] = 1; v[i] = w[i] = 0;
  }
  for (i=0; i<10; ++i) {
    AtAu(u,v,w);
    AtAu(v,u,w);
  }
  for (i=0; i<n; ++i) {
    vBv += u[i]*v[i];
    vv  += v[i]*v[i];
  }
  return Math.sqrt(vBv/vv);
}

var actual = '';
for (var i = 6; i <= 48; i *= 2) {
  actual += spectralnorm(i) + ',';
}
assertEq(actual, "1.2657786149754053,1.2727355112619148,1.273989979775574,1.274190125290389,");
