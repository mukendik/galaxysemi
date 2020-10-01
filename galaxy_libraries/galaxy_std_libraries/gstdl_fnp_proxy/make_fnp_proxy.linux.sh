# usage pass the destination library path the fnp toolkit dir and the fnp platforme type
# make_fnp_proxy.win.bat DESTLIB_DIR FNP_TOOLKITDIR FNP_PLATFORM
# PLATFORM can be i86_n3 for win32 and x64_n6 for win64  x64_lsb for linux64 and i86_lsb for linux32

export DESTLIB_DIR=$1

if [ x$DESTLIB_DIR != 'x' ]; then
    echo "DESTLIB_DIR = " $DESTLIB_DIR
else
    echo "-- Destination library not specified"
    exit 1
fi

if [ -d $DESTLIB_DIR ]
then
   export DESTLIB_DIR
else
   echo "--  Destination library does not exist"
   exit 1
fi

export FNP_TOOLKITDIR=$2

if [ x$FNP_TOOLKITDIR != 'x' ]; then
    echo "FNP_TOOLKITDIR = " $FNP_TOOLKITDIR
else
   echo "-- FNP TOOLKIT DIR library not specified"
   exit 1
fi

if [ -d $FNP_TOOLKITDIR ]; then
    export FNP_TOOLKITDIR
else
    echo "--  FNP TOOLKIT DIR does not exist"
   exit 1
fi

export FNP_PLATFORM=$3

if [ x$FNP_PLATFORM != 'x' ]; then
    echo "FNP_PLATFORM = " $FNP_PLATFORM
else
   echo "-- FNP TOOLKIT PLLATFORM not specified can be i86_n3 for win32 and x64_n6 for win64  x64_lsb for linux64 and i86_lsb for linux32"
   exit 1
fi

if [ -d ${FNP_TOOLKITDIR}/${FNP_PLATFORM} ]; then
    export FNP_PLATFORM
else
    echo "-- FNP TOOLKIT PLATFORM DIR does not exist"
    exit 1
fi

set PATH= ${FNP_TOOLKITDIR}/${FNP_PLATFORM}:${PATH}

export CURRENT_DIR=`pwd`
cd  ${CURRENT_DIR}

export MAKEFILE_ACT=make_fnp_proxy.act.${FNP_PLATFORM}

if [ -f $MAKEFILE_ACT ]; then
    export MAKEFILE_ACT
else
    echo "-- Makefile to build the proxy lib can not be found must be ${MAKEFILE_ACT}"
    exit 1
fi

make -f ${MAKEFILE_ACT} FNP_PROVIDER_DIR_M=${CURRENT_DIR} FNP_TOOLKITDIR_M=${FNP_TOOLKITDIR} DESTLIB_DIR_M=${DESTLIB_DIR} FNP_PLATFORM_M=${FNP_PLATFORM}
rc=$?
if [ $rc -ne 0 ] ; then
    echo "make command failure (${rc})" >&2
    exit $rc
fi

cp -f ${CURRENT_DIR}/sources/fnp_proxy_lx.xml ${FNP_TOOLKITDIR}/${FNP_PLATFORM}

cd   ${FNP_TOOLKITDIR}/${FNP_PLATFORM}
./preptool -v fnp_proxy_lx.xml
if [ $rc -ne 0 ] ; then
    echo "preptool command failure (${rc})" >&2
    exit $rc
fi

cp -f fnp.so fnp_libFNP.so ${DESTLIB_DIR}

cd  ${CURRENT_DIR}
