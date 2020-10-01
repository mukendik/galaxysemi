#!/bin/bash
# usage pass the destination library path the fnp toolkit dir and the fnp\
# platforme type
# make_fnp_proxy.win.bat DESTLIB_DIR FNP_TOOLKITDIR FNP_PLATFORM
# PLATFORM can be i86_n3 for win32 and x64_n6 for win64  x64_lsb for\
# linux64 and i86_lsb for linux32

# export DEVDIR & BRANCH
# export DEVDIR=~/prod/gex-prod-V7.1 ; export BRANCH=V7.1 // export DEVDIR=~/prod/gex-prod-master ; export BRANCH=master
# $1 --> $DEVDIR/galaxy_products/gex_product/bin
# $2 --> ~/fnp_toolkit_$BRANCH
# export DEVDIR=~/prod/gex-prod-master ; export BRANCH=master ; ./make_fnp_proxy.sh $DEVDIR/galaxy_products/gex_product/bin ~/fnp_toolkit_$BRANCH

export kernel=`uname`
export DESTLIB_DIR=$1
export FNP_TOOLKITDIR=$2
export CURRENT_DIR=`pwd`
export conf=~/.gex-prod-conf.sh

if [ -f $conf ]; then
    source $conf
else
    echo "warning: `basename $0`: cannot find $conf"
fi


if [ x$DESTLIB_DIR != 'x' ]; then
    echo "DESTLIB_DIR = " $DESTLIB_DIR
else
    echo "-- Destination library not specified"
    exit
fi

if [ -d $DESTLIB_DIR ]
then
    export DESTLIB_DIR
else
    echo "--  Destination library does not exist"
    exit
fi

if [ x$FNP_TOOLKITDIR != 'x' ]; then
    echo "FNP_TOOLKITDIR = " $FNP_TOOLKITDIR
else
    echo "-- FNP TOOLKIT DIR library not specified"
    exit
fi

if [ -d $FNP_TOOLKITDIR ]
then
    export FNP_TOOLKITDIR
else
    echo "--  FNP TOOLKIT DIR does not exist"
    exit
fi

if [ $kernel = "Linux" ]; then
    if [ `uname -m` = "x86_64" ]; then
          export FNP_PLATFORM="x64_lsb"
    else
          export FNP_PLATFORM="i86_lsb"
    fi
    if [ -d ${FNP_TOOLKITDIR}/${FNP_PLATFORM} ]; then
        export FNP_PLATFORM
    else
        echo "-- FNP TOOLKIT PLATFORM DIR does not exist"
        exit
    fi
    set PATH= ${FNP_TOOLKITDIR}/${FNP_PLATFORM}:${PATH}
    cd  ${CURRENT_DIR}
    export MAKEFILE_ACT=make_fnp_proxy.act.${FNP_PLATFORM}
    if [ -f $MAKEFILE_ACT ]; then
         export MAKEFILE_ACT
    else
        echo "-- Makefile to build the proxy lib can not be found must be ${MAKEFILE_ACT}"
        exit
    fi
    make -f ${MAKEFILE_ACT} FNP_PROVIDER_DIR_M=${CURRENT_DIR} FNP_TOOLKITDIR_M=${FNP_TOOLKITDIR} DESTLIB_DIR_M=${DESTLIB_DIR} FNP_PLATFORM_M=${FNP_PLATFORM}
    \cp -f ${CURRENT_DIR}/sources/fnp_proxy_lx.xml ${FNP_TOOLKITDIR}/${FNP_PLATFORM}
    cd ${FNP_TOOLKITDIR}/${FNP_PLATFORM}
    ./preptool -v fnp_proxy_lx.xml
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/bin" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/bin
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/docs" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/docs
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/FNPLicensingService" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/FNPLicensingService
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/galaxylm" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/galaxylm
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/lmadmin" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/lmadmin
    fi  
    \cp -f fnp.so fnp_libFNP.so ${DESTLIB_DIR}
    \cp -f ${DESTLIB_DIR}/galaxy-la ${DESTLIB_DIR}/GS-License-Activation-Utility/bin/
    \cp -f appactutil appactutil_libFNP.so FnpCommsSoap.so serveractutil serveractutil_libFNP.so tsreset_app tsreset_app_libFNP.so tsreset_svr tsreset_svr_libFNP.so publisher/install_fnp.sh ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/
    \cp -Rf publisher/FNPLicensingService ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/
    \cp -f lmadmin-i86_lsb-11_11_1_1.bin ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/lmadmin/
    \cp -f galaxylm galaxylm_libFNP.so ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/galaxylm/
    cd ${CURRENT_DIR}

elif [ $kernel = "Darwin" ]; then
    export FNP_PLATFORM="universal_mac10"
    if [ -d ${FNP_TOOLKITDIR}/${FNP_PLATFORM} ]; then
        export FNP_PLATFORM
    else
        echo "-- FNP TOOLKIT PLATFORM DIR does not exist"
        exit
    fi
    set PATH= ${FNP_TOOLKITDIR}/${FNP_PLATFORM}:${PATH}
    cd  ${CURRENT_DIR}
    export MAKEFILE_ACT=make_fnp_proxy.act.${FNP_PLATFORM}
    if [ -f $MAKEFILE_ACT ]; then
         export MAKEFILE_ACT
    else
        echo "-- Makefile to build the proxy lib can not be found must be ${MAKEFILE_ACT}"
        exit
    fi
    #sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer/
    export PATH=/Applications/Xcode.app/Contents/Developer/usr/bin:$PATH
    make -f ${MAKEFILE_ACT} FNP_PROVIDER_DIR_M=${CURRENT_DIR} FNP_TOOLKITDIR_M=${FNP_TOOLKITDIR} DESTLIB_DIR_M=${DESTLIB_DIR} FNP_PLATFORM_M=${FNP_PLATFORM}
    \cp -f -f ${CURRENT_DIR}/fnp.dylib ${FNP_TOOLKITDIR}/${FNP_PLATFORM}
    \cp -f ${CURRENT_DIR}/sources/fnp_proxy_mac10.xml ${FNP_TOOLKITDIR}/${FNP_PLATFORM}
    cd ${FNP_TOOLKITDIR}/${FNP_PLATFORM}
    ./preptool -v fnp_proxy_mac10.xml
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/bin" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/bin
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/docs" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/docs
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/FNPLicensingService" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/FNPLicensingService
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/galaxylm" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/galaxylm
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/lmadmin" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/lmadmin
    fi  
    \cp -f fnp.dylib fnp_libFNP.dylib ${DESTLIB_DIR}
    \cp -Rf ${DESTLIB_DIR}/galaxy-la.app ${DESTLIB_DIR}/GS-License-Activation-Utility/bin/
    \cp -f appactutil appactutil_libFNP.dylib FnpCommsSoap.dylib serveractutil serveractutil_libFNP.dylib tsreset_app tsreset_app_libFNP.dylib tsreset_svr tsreset_svr_libFNP.dylib publisher/install_fnp.sh ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils
    \cp -Rf publisher/FNPLicensingService ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/
    \cp -f lmadmin-universal_mac10-11_11_1_1.zip ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/lmadmin
    \cp -f galaxylm galaxylm_libFNP.dylib ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/galaxylm
    cd ${CURRENT_DIR}

else
    if echo $MINGWDIR | grep 64 >/dev/null; then
          export FNP_PLATFORM="x64_n6"
    else
          export FNP_PLATFORM="i86_n3"
    fi
    if [ -d ${FNP_TOOLKITDIR}/${FNP_PLATFORM} ]; then
        export FNP_PLATFORM
    else
        echo "-- FNP TOOLKIT PLATFORM DIR does not exist"
        exit
    fi
    set PATH= ${FNP_TOOLKITDIR}/${FNP_PLATFORM}:${PATH}
    cd  ${CURRENT_DIR}
    export MAKEFILE_ACT=make_fnp_proxy.act.${FNP_PLATFORM}
    if [ -f $MAKEFILE_ACT ]; then
         export MAKEFILE_ACT
    else
        echo "-- Makefile to build the proxy lib can not be found must be ${MAKEFILE_ACT}"
        exit
    fi
    #nmake -f ${MAKEFILE_ACT} FNP_PROVIDER_DIR_M=${CURRENT_DIR} FNP_TOOLKITDIR_M=${FNP_TOOLKITDIR} DESTLIB_DIR_M=${DESTLIB_DIR} FNP_PLATFORM_M=${FNP_PLATFORM}
    \cp -f ${CURRENT_DIR}/sources/fnp_proxy.xml ${FNP_TOOLKITDIR}/${FNP_PLATFORM}
    cd ${FNP_TOOLKITDIR}/${FNP_PLATFORM}
    ./preptool -v fnp_proxy.xml
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/bin" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/bin
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/docs" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/docs
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/FNPLicensingService" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/FNPLicensingService
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/galaxylm" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/galaxylm
    fi
    if [ ! -d "${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/lmadmin" ]; then
         mkdir ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/lmadmin
    fi  
    #\cp -f fnp.dll fnp_libFNP.dll FNP_Act_Installer.dll ${DESTLIB_DIR}
    \cp -f ${DESTLIB_DIR}/galaxy-la.exe ${DESTLIB_DIR}/GS-License-Activation-Utility
    \cp -f appactutil.exe appactutil_libFNP.dll FNP_Act_Installer.dll FnpCommsSoap.dll installanchorservice.exe serveractutil.exe serveractutil_libFNP.dll tsreset_app.exe tsreset_app_libFNP.dll tsreset_svr.exe tsreset_svr_libFNP.dll ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils
    \cp -f lmadmin-i86_n3-11_11_1_1.exe ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/lmadmin
    \cp -f galaxylm.exe galaxylm_libFNP.dll ${DESTLIB_DIR}/GS-License-Activation-Utility/fnp_utils/galaxylm
    cd ${CURRENT_DIR}
fi
