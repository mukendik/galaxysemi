#!/bin/sh
if [ $# -eg 1 ]; then cd $1; fi
find . -type l |xargs tar -zcvf symlinks.tar.gz
find . -type l |xargs rm -f
