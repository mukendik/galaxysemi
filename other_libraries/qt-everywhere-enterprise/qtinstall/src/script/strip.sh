#/bin/sh
for f in $(find . -name "*.so.5.2.1"); do strip $f; done