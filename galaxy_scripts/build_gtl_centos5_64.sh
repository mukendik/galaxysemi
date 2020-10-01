#!/bin/sh
set -xe
if [ -z "${BRANCH}" ]; then
    cd ..
    echo "BRANCH not defined! pwd to ${PWD}/.."
else
    cd ~/prod/gex-prod-${BRANCH}
fi
export DIST_NAME_LONG=centos5_64
export DISTGROUP=centos5_64
export distName=linux64
export GTL_GROUP=lg3
export JAVA_HOME=/home/gexprod/install/java
export LOAD_MAX=2.5
export MAVEN_HOME=/home/gexprod/install/maven
export MVN_OPTIONS="-Dlinux -Dlinux64 -Dcentos5_64"
export QTDIR=/data/qt/5.2.1
export QTSRCDIR=/data/qt/qt-everywhere-enterprise-src-5.2.1
export PATH=${MAVEN_HOME}/bin:${JAVA_HOME}/bin:/usr/kerberos/bin:/usr/share/apache-maven/bin:/usr/local/bin:/usr/bin:/home/gexprod/bin:/bin:${QTDIR}/bin
export SLAVE_HOME=/home/gexprod/jenkins

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
  -Denv.NO_ERROR_ON_WARNING=1 -DnoMainBuild -DnoUnitTests -DnoGexPackages
mvn -B -e ${MVN_OPTIONS} ${mvnX} -pl :gtl,:gstdl deploy \
  -Denv.NO_ERROR_ON_WARNING=1 -DnoMainBuild -DnoUnitTests -DnoGexPackages
