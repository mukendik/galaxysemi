#!/bin/bash
# script for starting QtCreator with Gex environnement paths variables set

# Example with prefix :
#   cd ~/dev/galaxy_scripts
#   ./LaunchQtCreatorForGex.sh ~
export LC_ALL=C
echo $SHELL
if [ -n "$1" ]; then
    prefix=$1
else
    prefix=/opt
fi

git_branch=`git rev-parse --abbrev-ref HEAD`
echo "GIT branch is ${git_branch}"

# .qtcreator.conf has to be located in you user folder. It is used to configure your own 
# path for QTDIR, QTSRCDIR and QTCREATOR folders
# It takes one parameter as argument which is used to determine which version we are working on.
# If it doesn't exist or empty, default behavior will apply
# Below is an example:
# if [ $1 == "master" ] || [ $1 == "V7.3" ]
# then 
#     export CUSTOM_QTCREATOR=/usr/local/opt/Qt/5.2
#	  export CUSTOM_QTDIR=/usr/local/opt/Qt/5.2/5.2.1/clang_64
#	  export CUSTOM_QTSRCDIR=/usr/local/opt/Qt/5.2/5.2.1/Src
# elif [ $1 == "V7.2" ]
# then
#	  export CUSTOM_QTCREATOR=/usr/local/opt/Qt/5.1
#	  export CUSTOM_QTDIR=/usr/local/opt/Qt/5.1/5.1.1/clang_64
#	  export CUSTOM_QTSRCDIR=/usr/local/opt/Qt/5.1/5.1.1/Src
# fi
if [ -e ~/.qtcreator.conf ]
then
	echo "Sourcing ~/.qtcreator.conf"
	source ~/.qtcreator.conf ${git_branch}
fi

# DEVDIR : where all the sources are
export DEVDIR=$PWD/..
echo "DEVDIR set to " $DEVDIR

GEX_PATH=${DEVDIR}/galaxy_products/gex_product/bin
export GEX_PATH

PATH=/bin:/usr/bin:/usr/local/bin:/usr/X11R6/bin:.:${GEX_PATH}:${PATH}
export PATH
echo "PATH set to " $PATH

export QTDIR=''

# QTDIR : where the QT SDK is
if [ -z ${CUSTOM_QTDIR+x} ];
then
	if [ -d /usr/local/Trolltech/Qt-4.7.3 ] 
	then
 		export QTDIR=/usr/local/Trolltech/Qt-4.7.3/qt
	fi

	if [ -d $prefix/include ] 
	then
 		export QTDIR=$prefix
	fi

	if [ -d /opt/QtSDK ] 
	then
 		export QTDIR=/opt/QtSDK
	fi

	if [ -d /opt/QtSDK/Desktop/Qt/4.8.1/gcc ]
	then
 		export QTDIR=/opt/QtSDK/Desktop/Qt/4.8.1/gcc
	fi

	if [ -d $HOME/Desktop/qt-everywhere-opensource-src-4.8.3 ]
	then
 		export QTDIR=$HOME/Desktop/qt-everywhere-opensource-src-4.8.3
	fi

	if [ -d /data/qt/4.8.4 ]
	then
 		export QTDIR=/data/qt/4.8.4
	fi

	if [ -d /opt/Qt/5.2.1 ]
	then
 		export QTDIR=/opt/Qt/5.2.1
	fi
else
	if [ -d $CUSTOM_QTDIR ] 
	then
		export QTDIR=$CUSTOM_QTDIR
	fi
fi

if [ x$QTDIR != 'x' ]; then
 echo "QTDIR = " $QTDIR
else
 echo "Error : no QT SDK found ! Please install the Qt SDK (with at least Desktop Qt 4.8.1) in $prefix/"
 exit
fi

$QTDIR/bin/qmake -v

#QTSRCDIR : where the corresponding src of qt is
export QTSRCDIR=$QTDIR/src

if [ -z ${CUSTOM_QTSRCDIR+x} ];
then
	if [ -d /opt/QtSDK/QtSources/4.8.1/src ]
	then
 		export QTSRCDIR=/opt/QtSDK/QtSources/4.8.1/src
	fi

	if [ -d $HOME/Bureau/Development/qt-everywhere-opensource-src-4.8.4 ]
	then
 		export QTSRCDIR=$HOME/Bureau/Development/qt-everywhere-opensource-src-4.8.4/src
	fi
else
	if [ -d $CUSTOM_QTSRCDIR ] 
	then
		export QTSRCDIR=$CUSTOM_QTSRCDIR
	fi
fi

if [ -n $QTSRCDIR ]
then
 echo "QTSRCDIR = " $QTSRCDIR
else
 echo "Warning : no Qt src dir found in Qt sdk. Building will probably fail !"
fi

system=`uname`

if [ $system == "Darwin" ]; then
    osFolder=mac
else
    platform=`uname -i`
    osFolder=linux64
    if [ "$platform" == "x86_64" ]; then
    	 COMPIL_CONFIG=$(zenity --list --height=300 --radiolist --column "Selection" --column "Compilation mode" FALSE 32-bits TRUE 64-bits)

        if [ $COMPIL_CONFIG = '32-bits' ]; then
	    osFolder=linux32
        elif [ $COMPIL_CONFIG = '64-bits' ]; then
	    osFolder=linux64
        fi
   fi
fi

echo osFolder = $osFolder
echo system = $system

LIBRARY_PATH=\
$QTDIR/bin:\
$QTDIR/lib:\
${DEVDIR}/galaxy_libraries/galaxy_std_libraries/lib/$osFolder:\
${DEVDIR}/galaxy_libraries/galaxy_qt_libraries/lib/$osFolder:\
${DEVDIR}/other_libraries/libqglviewer/lib/$osFolder:\
${DEVDIR}/galaxy_products/gex_product/gex-pb/lib/$osFolder:\
${DEVDIR}/galaxy_products/gex_product/gex-oc/lib/$osFolder:\
${DEVDIR}/galaxy_products/gex_product/gex-log/lib/$osFolder:\
${DEVDIR}/galaxy_products/gex_product/plugins/lib/$osFolder:\
${DEVDIR}/other_libraries/qtpropertybrowser-2.5_1-commercial/lib/$osFolder:\
${DEVDIR}/other_libraries/R/lib/$osFolder

# set R_HOME
export GALAXY_R_HOME=${DEVDIR}/other_libraries/R/r_home/$osFolder
echo "GALAXY_R_HOME set to " $GALAXY_R_HOME

#if [ $platform = "x86_64" ]; then
if [ $osFolder == "linux64" ]; then
    LIBRARY_PATH=$LIBRARY_PATH:\
                 ${DEVDIR}/other_libraries/chartdirector/lib/linux64
else
    LIBRARY_PATH=$LIBRARY_PATH:${DEVDIR}/other_libraries/chartdirector/lib/$osFolder
fi

LIBRARY_PATH=$LIBRARY_PATH:${DEVDIR}/other_libraries/fnp-toolkit/lib/$osFolder

set LANG=en

if [ $system == "Darwin" ]; then
    LIBRARY_PATH=$LIBRARY_PATH:${DEVDIR}/other_libraries/tufao/0.8/lib/$osFolder
    LIBRARY_PATH=$LIBRARY_PATH:${DEVDIR}/other_libraries/quazip-0.4.4/lib/$osFolder
    export DYLD_LIBRARY_PATH=$LIBRARY_PATH 
    echo "LIBRARY_PATH set to " $DYLD_LIBRARY_PATH
else
    LIBRARY_PATH=$LIBRARY_PATH:${DEVDIR}/galaxy_products/gex_product/bin/lib
    export LD_LIBRARY_PATH=$LIBRARY_PATH
    echo "LIBRARY_PATH set to " $LD_LIBRARY_PATH
fi

echo "This version should be built with gcc 4.4..."
gcc --version

if [ -z "$GEX_LOGLEVEL" ]; then
    export GEX_LOGLEVEL=7
fi
echo "GEX_LOGLEVEL=" $GEX_LOGLEVEL
echo "GEXOC_LOGLEVEL=" $GEXOC_LOGLEVEL
echo "GEXPB_LOGLEVEL=" $GEXPB_LOGLEVEL

# launch  qtcreator (is it better to use the one synchronized with the SDK version ?)
#$prefix/qtsdk-2010.02/bin/qtcreator
#$prefix/qtsdk-2009.01/bin/qtcreator

#if [ -d $prefix/qtcreator-2.1.0 ]; then
# echo "Launching QtCreator 2.1.0..."
# $prefix/qtcreator-2.1.0/bin/qtcreator
#else

if [ -z ${CUSTOM_QTCREATOR+x} ]
then 
	if [ -d /opt/Qt/Tools/QtCreator/bin ]
	then
	echo "Press enter to launch QtCreator as in /opt/Qt/Tools/QtCreator..."
	read t
	/opt/Qt/Tools/QtCreator/bin/qtcreator master
	elif [ -d /opt/qtcreator-2.4.1 ]
	then
    	echo "Press enter to launch QtCreator 2.4.1 session master..."
    	read t
    	/opt/qtcreator-2.4.1/bin/qtcreator master
	elif [ -d /Users/gexprod/QtSDK ]
	then
    	echo "Press enter to launch QtCreator 2.4.1 session master.."
    	read t
    	open -a /Users/gexprod/QtSDK/Qt\ Creator.app
	elif [ -d /home/gexdev/Bureau/Applications/QtCreator-2.7.0 ]
	then
    	echo "Press enter to launch QtCreator 2.4.1 session master..."
    	/home/gexdev/Bureau/Applications/QtCreator-2.7.0/bin/qtcreator
    elif [ -d /opt/Qt/QT\ Creator.app/Contents/MacOS ]
 	then
    	echo "Press enter to launch QtCreator session master..."    
		/opt/Qt/Qt\ Creator.app/Contents/MacOS/Qt\ Creator master
 	else
 		echo "QtCreator 2.4.1 not found. Launching registered QtCreator if any... "
 		qtcreator -version
 		read t
 		qtcreator
	fi
else
	if [ -d $CUSTOM_QTCREATOR ] 
	then
	    if [ $system == "Darwin" ]; then
			echo "Launching QtCreator from " $CUSTOM_QTCREATOR 
    		$CUSTOM_QTCREATOR/Qt\ Creator.app/Contents/MacOS/Qt\ Creator &
        else
            echo "Launching QtCreator from " $CUSTOM_QTCREATOR 
    		$CUSTOM_QTCREATOR/bin/qtcreator
        fi
	fi
fi
