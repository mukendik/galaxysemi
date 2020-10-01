#!/bin/sh
export ARCHIVE_EXT=tar.gz
export DIST_NAME_LONG=centos6_64
export DISTGROUP=centos6_64
export distName=linux64
export GTL_GROUP=centos6_64
export LIB_MYSQLCLIENT_DIR=/usr/lib64/mysql
export MVN_OPTIONS="-Dlinux -Dlinux64 -Dcentos6_64"
export QTDIR=/home/cigone/install/Qt-5.2.1/5.2.1/gcc_64
export QTSRCDIR=/home/cigone/install/Qt-5.2.1/5.2.1/Src
mvn install ${MVN_OPTIONS} -fae -T 4

