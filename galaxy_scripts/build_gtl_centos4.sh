#!/bin/sh
set -xe
if [ -z "${BRANCH}" ]; then
    cd ..
    echo "BRANCH not defined! pwd to ${PWD}/.."
else
    cd ~/prod/gex-prod-${BRANCH}
fi

source ~/.keychain/*-sh &>~/tmp/keychain.log
git pull

export DIST_NAME_LONG=centos4_32
export DISTGROUP=centos4_32
export distName=linux32
export GTL_GROUP=lg1
export JAVA_HOME=/home/gexprod/install/java
export LOAD_MAX=2.5
export MAVEN_HOME=/home/gexprod/install/maven
export MVN_OPTIONS="-Dlinux -Dlinux32 -Dcentos4_32"
export PACKAGE_DEST_V74=/home/gexprod/prod/gex-prod-master/galaxy_products/gex_product/buildpackage/packages/V7.4
export PATH=${MAVEN_HOME}/bin:${JAVA_HOME}/bin:/usr/kerberos/bin:/usr/share/apache-maven/bin:/usr/local/bin:/usr/bin:/home/gexprod/bin:/bin:/data/qt/4.8.3/bin
export QTDIR=/data/qt/4.8.3
export QTSRCDIR=/data/qt/qt-everywhere-commercial-src-4.8.3/src
export SLAVE_HOME=/home/gexprod/jenkins
export GTL_GROUP=lg1

mvnClean=clean
mvnCleanAfterBuild=clean
#mvnX="-X -DX"
mvnX=""
export NO_ERROR_ON_WARNING=1


env
mvn -B ${MVN_OPTIONS} help:active-profiles -N

mvn -B -e ${MVN_OPTIONS} ${mvnX} clean

# BUILD GTL/GSTDL AND DEPLOY THEM
mvn -B -e ${MVN_OPTIONS} ${mvnX} -pl :gtl,:gstdl -am install \
  -DnoMainBuild -DnoUnitTests -DnoGexPackages \
  -Denv.NO_ERROR_ON_WARNING=1
mvn -B -e ${MVN_OPTIONS} ${mvnX} -pl :gtl,:gstdl deploy
