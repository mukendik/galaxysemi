#!/bin/bash
# script for 

export DEVDIR=$PWD/..
echo "DEVDIR set to " $DEVDIR

GEX_PATH=${DEVDIR}/galaxy_products/gex_product/bin_debug
export GEX_PATH

PATH=/bin:/usr/bin:/usr/local/bin:/usr/X11R6/bin:.:${GEX_PATH}:${PATH}
export PATH
echo "PATH set to " $PATH

export LD_LIBRARY_PATH=\
$QTDIR/bin:\
$QTDIR/lib:\
${DEVDIR}/other_libraries/libqglviewer/lib:\
${DEVDIR}/galaxy_products/gex_product/gex-pb/lib_debug:\
${DEVDIR}/galaxy_products/gex_product/gex-oc/lib_debug:\
${DEVDIR}/galaxy_products/gex_product/bin_debug

ldd ${DEVDIR}/galaxy_products/gex_product/bin_debug/gex.exe

