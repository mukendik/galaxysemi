#!/bin/bash
#set -x

# Set platform-dependant variables
SetPlatformEnv()
{
    Platform=`uname -s`
    if [ ${Platform} = "SunOS" ]; then
        DEVDIR=${HOME}/dev/galaxy_dev_v64
        QTDIR=/usr/local/Trolltech/Qt-4.7.2
        QTSRCDIR=/opt/qt/4.7.2/src
        Makespec=solaris-g++
        PlatformPrefix=sol
        PlatformDir=solaris
        OsType=unix
        MakeTool=make
        MYSQL_QMAKE_ARGS="\"INCLUDEPATH+=/opt/mysql/mysql/include\" \"LIBS+=-L/opt/mysql/mysql/lib\" \"LIBS+=-lmysqlclient_r\""
        OCI_QMAKE_ARGS=
    elif [ ${Platform} = "Darwin" ]; then
        DEVDIR=${HOME}/gexprod/galaxy_dev
        PlatformPrefix=mac
        PlatformDir=mac
        OsType=unix
        MakeTool=make
        MYSQL_QMAKE_ARGS="\"INCLUDEPATH+=/usr/share/mysql-connector-c-6.0.2-osx10.5-x86-64bit/include\""
        MYSQL_QMAKE_ARGS="$MYSQL_QMAKE_ARGS \"LIBS+=-lmysqlclient_r\""
        kernel=`uname -r`
        kernelRevision=`echo "$kernel" | awk -F- '{ print $1 }'`
        kernelMinorRevision=`echo "$kernelRevision" | awk -F. '{ print $3 }'`
        if [ "$kernelMinorRevision" -gt 23 ]; then
            MYSQL_QMAKE_ARGS="$MYSQL_QMAKE_ARGS \"LIBS+=-L/usr/lib\""
        else
            MYSQL_QMAKE_ARGS="$MYSQL_QMAKE_ARGS \"LIBS+=-L/usr/share/mysql-connector-c-6.0.2-osx10.5-x86-64bit/lib\""
        fi
    elif [ ${Platform} = "Linux" ]; then
        DEVDIR=${HOME}/dev/galaxy_dev_v64
        QTDIR=/usr/local/Trolltech/Qt-4.7.2
        QTSRCDIR=/opt/qt/4.7.2/src
        Makespec=linux-g++
        PlatformPrefix=linux
        PlatformDir=linux
        OsType=unix
        MakeTool=make
        MYSQL_QMAKE_ARGS="\"INCLUDEPATH+=/usr/include/mysql\""
        MYSQL_QMAKE_ARGS="$MYSQL_QMAKE_ARGS \"LIBS+=-lmysqlclient_r\""
        kernel=`uname -r`
        kernelRevision=`echo "$kernel" | awk -F- '{ print $1 }'`
        kernelMinorRevision=`echo "$kernelRevision" | awk -F. '{ print $3 }'`
        if [ "$kernelMinorRevision" -gt 23 ]; then
            MYSQL_QMAKE_ARGS="$MYSQL_QMAKE_ARGS \"LIBS+=-L/usr/lib\""
        else
            MYSQL_QMAKE_ARGS="$MYSQL_QMAKE_ARGS \"LIBS+=-L/usr/lib/mysql\" \"LIBS+=-ldl\" \"LIBS+=-lrt\""
        fi
    else
        DEVDIR=/i/galaxy_dev_v64
        QTDIR=/c/Qt/4.7.2
        MINGWDIR=/c/Qt/2010.05/mingw/bin
        QTSRCDIR=${QTDIR}/src
        Makespec=win32-g++
        PlatformPrefix=win32
        PlatformDir=win32
        OsType=win32
        MakeTool=mingw32-make
        PATH=${MINGWDIR}:${PATH}
        MYSQLPATH=/c/MySQL
    fi

    # Configuration for production (before PATH setting)
    cd `dirname "$1"`/..
    DEVDIR=`pwd -P`
    cd -
    if [ -f ~/.gex-prod-conf.sh ]; then
        source ~/.gex-prod-conf.sh
    fi
    if [ $Platform != "Linux" ] &&
       [ $Platform != "SunOS" ] &&
       [ $Platform != "Darwin" ]; then
        if echo $MINGWDIR | grep 64 >/dev/null; then
            MYSQL_QMAKE_ARGS="INCLUDEPATH+=c:/mysql/mysql-connector-c-6.0.2/include LIBS+=c:/mysql/mysql-connector-c-6.0.2/lib/opt/libmysql.a"
            OCI_QMAKE_ARGS="\"INCLUDEPATH+=c:/oracle/product/11.2.0/client_2/OCI/include\" \"LIBS+=-Lc:/oracle/product/11.2.0/client_2/OCI/lib/MSVC\""
        else
            MYSQL_QMAKE_ARGS="\
INCLUDEPATH+=c:/mysql/mysql-connector-c-6.1.2/include \
LIBS+=c:/mysql/mysql-connector-c-6.1.2/lib/libmysql.lib"
            OCI_QMAKE_ARGS="\"INCLUDEPATH+=c:/oracle/product/10.2.0/client_1/OCI/include\" \"LIBS+=-Lc:/oracle/product/10.2.0/client_1/OCI/lib/MSVC\""
        fi
    elif [ $Platform = "Linux" ]; then
        if ! test -d "$ORACLE_HOME"; then
            echo "warning: ORACLE_HOME is not a directory"
        else
            OCI_QMAKE_ARGS="\"INCLUDEPATH+=$ORACLE_HOME/rdbms/public\""
            OCI_QMAKE_ARGS="$OCI_QMAKE_ARGS \"LIBS+=-L$ORACLE_HOME/lib\""
        fi
    elif [ $Platform = "Darwin" ]; then
        if ! test -d "$ORACLE_HOME"; then
            echo "warning: ORACLE_HOME is not a directory"
        else
            OCI_QMAKE_ARGS="\"INCLUDEPATH+=$ORACLE_HOME/sdk/include\""
            OCI_QMAKE_ARGS="$OCI_QMAKE_ARGS \"LIBS+=-L$ORACLE_HOME\""
        fi
    fi
    if [ -d $QTSRCDIR/qtbase ]; then
        GEX_REMINDER_FILE=$QTSRCDIR/qtbase/src/sql/drivers/oci/gex_reminder.txt
    else
        GEX_REMINDER_FILE=$QTSRCDIR/sql/drivers/oci/gex_reminder.txt
    fi

	PATH=${QTDIR}/bin:${PATH}
        PATH=${MINGWDIR}:${PATH}
	export DEVDIR QTDIR PATH QTSRCDIR INCLUDE LIB
}

# Log function (logs to screen and file)
Log()
{
	echo `date`: ${1} | tee -a ${LogFile}
}

# Make function
Make_Project()
{
    Log ""
    Log "======================================================================"
    Log " Project ${1}"
    Log "======================================================================"
    if [ ${1} = "oci" ]; then
	# Check if Gex reminder file is present
        if [ ! -f ${GEX_REMINDER_FILE} ]; then
            echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
            echo "!! The GEX Reminder file is Missing!!                      !!"
            echo "!! You have to modify the qsql_oci.cpp file to add the     !!"
            echo "!! possibility to login as SYSDBA, then create the GEX     !!"
            echo "!! Reminder file.                                          !!"
            echo "!!                                                         !!"
            echo "!! For a description of required changes, please check     !!"
            echo "!! the GEX reminder template file.                         !!"
            echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
            echo "GEX Reminder file         : ${GEX_REMINDER_FILE}"
            echo "GEX Reminder template file: ${DEVDIR}/galaxy_scripts/gex_reminder_template.txt"
            exit
        fi
    fi
    cd ${2}
    Log "pwd = `pwd`"
    Log " ==> GENERATING Makefile (qmake)..."
    args="$QMAKE_ARGS -spec $Makespec -o Makefile_$PlatformPrefix $1.pro"
    Log "qmake $args"
    eval qmake $args 2>&1 | tee -a $LogFile
    if [ ${CleanProjects} = "true" ]; then
        Log " ==> CLEANING..."
        Log ""
        ${MakeTool} -f Makefile_${PlatformPrefix} clean 2>&1
        Log ""
    fi
    Log " ==> COMPILING (make)..."
    Log "pwd = "`pwd`
    Log ""
    $MakeTool -f Makefile_$PlatformPrefix 2>&1 | tee -a $LogFile
    Log ""
}

# Menu to get user choice
Menu()
{
    echo ""
    echo "Usage: make_sqldrivers [clean]"
    echo ""
    echo "1. Make MySQL sqldrivers"
    echo "2. Make OCI sqldrivers"
    echo "3. Make all"
    echo "4. Exit"
    echo ""
    printf "Enter your choice: "
    read MenuChoice
    ValidChoice=OK
    MakeMysqlSqldriver=false
    MakeOciSqldriver=false
    case ${MenuChoice} in
        1)	MakeMysqlSqldriver=true;;
        2)	MakeOciSqldriver=true;;
        3)	MakeMysqlSqldriver=true
		MakeOciSqldriver=true;;
    	4)	;;
    	*)	ValidChoice=NOK
    esac
}

###########################################################################
# Script begins here!!
###########################################################################
# Display menu and check user choice
ValidChoice=NOK
Menu
while [ ${ValidChoice} = "NOK" ]; do
    echo ""
    echo "Invalid choice: ${MenuChoice}"
    Menu
done

echo ""
echo "Your choice: ${MenuChoice}"

# Exit?
if [ ${MenuChoice} = "4" ]; then
    exit
fi

# Set platform-dependant variables
SetPlatformEnv "$0"

# Check if projects should be cleaned first
CleanProjects=false
if [ ${1}_ok = "clean_ok" ]; then
    CleanProjects=true
fi

# Make projects
#CurDir=${DEVDIR}/galaxy_scripts
CurDir=`pwd`
LogFile=${CurDir}/make_sqldrivers.txt
rm ${LogFile} 2> /dev/null

# MySQL Sqldriver ?
if [ ${MakeMysqlSqldriver} = "true" ]; then
    Log ""
    Log "##################### COMPILING MYSQL SqlDriver #####################"
    QMAKE_ARGS=${MYSQL_QMAKE_ARGS}
    if [ -d $QTSRCDIR/qtbase ]; then
        Make_Project mysql $QTSRCDIR/qtbase/src/plugins/sqldrivers/mysql
    else
        Make_Project mysql $QTSRCDIR/plugins/sqldrivers/mysql
    fi
fi

# OCI SqlDriver ?
if [ ${MakeOciSqldriver} = "true" ]; then
    Log ""
    Log "###################### COMPILING OCI SqlDriver ######################"
    QMAKE_ARGS=${OCI_QMAKE_ARGS}
    if [ -d $QTSRCDIR/qtbase ]; then
        Make_Project oci $QTSRCDIR/qtbase/src/plugins/sqldrivers/oci
    else
        Make_Project oci $QTSRCDIR/plugins/sqldrivers/oci
    fi
fi

# Done: check if errors occured
Log ""
if [ `grep -c " Error " ${LogFile}`_ok != "0_ok" ]; then
    Log "Compilation done WITH ERRORS. Check ${LogFile}."
else
    Log "Compilation done WITH SUCCESS. Check ${LogFile}."
#	Log "Installing drivers (make install)."
#	cd ${QTSRCDIR}/plugins/sqldrivers/mysql
#	${MakeTool} -f Makefile_${PlatformPrefix} install 2>&1 | tee -a ${LogFile}
#	cd ${QTSRCDIR}/plugins/sqldrivers/oci
#	${MakeTool} -f Makefile_${PlatformPrefix} install 2>&1 | tee -a ${LogFile}
fi
Log ""




