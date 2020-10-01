curDir=$(pwd)
cd ..

export BITROCK_EXEC=/home/gexprod/install/builder-9.0.0/bin/builder
export LOAD_MAX=2
export DISTGROUP=centos6_64
export DIST_NAME_LONG=centos6_64
export QTDIR=/home/gexprod/qt/5.2.1
export QTSRCDIR=/home/gexprod/qt/qt-everywhere-enterprise-src-5.2.1

mvn clean install -Dlinux -Dlinux64 -Dcentos6_64 -Drpm -DnoBitrock -X -DX -Drpm -DnoExternalGtl -DnoUnitTests -DnoMainBuild -DnoGtlBuild | tee $curDir/mvn.log
cd $curDir

