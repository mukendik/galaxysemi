#!/bin/bash
#set -x

if [ -z "$DEVDIR" ]; then echo "DEVDIR undefined!"; exit 1; fi
if [ -z "$QTDIR" ]; then echo "QTDIR undefined!"; exit 1; fi
if [ -z "$QTSRCDIR" ]; then echo "QTSRCDIR undefined!"; exit 1; fi

# Set platform-dependant variables
MYSQL_QMAKE_ARGS="\"INCLUDEPATH+=/usr/include/mysql\""
MYSQL_QMAKE_ARGS="$MYSQL_QMAKE_ARGS \"LIBS+=-lmysqlclient_r\""

#MYSQL_QMAKE_ARGS="$MYSQL_QMAKE_ARGS \"LIBS+=-L/usr/lib\""
MYSQL_QMAKE_ARGS="$MYSQL_QMAKE_ARGS \"LIBS+=-L/usr/lib/mysql\" \"LIBS+=-ldl\" \"LIBS+=-lrt\""

PATH=${QTDIR}/bin:${PATH}
PATH=${MINGWDIR}:${PATH}
export DEVDIR QTDIR PATH QTSRCDIR INCLUDE LIB

# MySQL Sqldriver

QMAKE_ARGS=${MYSQL_QMAKE_ARGS}
# Make function
cd $QTSRCDIR/qtbase/src/plugins/sqldrivers/mysql

echo "pwd = `pwd`"
args="$QMAKE_ARGS -spec linux-g++ -o Makefile_linux mysql.pro"
echo "qmake $args"
eval qmake $args 2>&1

# clean project
#make -f Makefile_linux clean 2>&1

echo "pwd = "`pwd`
echo "make -f Makefile_linux"
make -f Makefile_linux 2>&1

# qmake
#[gexprod@vps35948 mysql]$ pwd
#/home/gexprod/jenkins/workspace/qt-everywhere-enterprise/label/centos6-64/qt-everywhere-enterprise/build/qt-everywhere-enterprise-src-5.3.2/qtbase/src/plugins/sqldrivers/mysql
#export PATH=$PATH:/home/gexprod/jenkins/workspace/qt-everywhere-enterprise/label/centos6-64/qt-everywhere-enterprise/build/qt-everywhere-enterprise-src-5.3.2/qtbase/bin/
#qmake -spec linux-g++ -o Makefile_linux mysql.pro