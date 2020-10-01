# ---------------------------------------------------------------------------- #
# make-unit-tests.sh
# ---------------------------------------------------------------------------- #
# ssh gexprod@ken.galaxysemi.com
# export UT=galaxy_unit_tests/galaxy_unit_test_1
# export BRANCH=master
# cd ~/jenkins/workspace/galaxy_dev/$BRANCH/label/*/galaxy_unit_tests
# export LABEL=`ls -1 ../..`
# export DEVDIR=~/jenkins/workspace/galaxy_dev/$BRANCH/label/$LABEL/$UT/target
# source ~/.gex-prod-conf.sh  # or:
# export QTDIR=~/qt/5.2.1
# export QTSRCDIR=~/qt/qt-everywhere-enterprise-src-5.2.1
# export Makespec=linux-g++-64
# ./make-unit-tests.sh --log=/dev/stdout $UT/`basename $UT`
# ---------------------------------------------------------------------------- #
timeout=60
export PATH=/usr/local/bin:/usr/bin:/bin
export LANG=C
fullargs="$*"

if [ "${1%=*}" = "--branch" ]; then
    export BRANCH=${1#*=}
    shift
fi
if [ -z "$BRANCH" ]; then
    echo -n "Usage: `basename $0` --branch=<branch> [--timeout=<sec>]"
    echo -n " [--log=<file>] [--debug] [--no-clean] [--no-make]"
    echo -n " [--valgrind|--gdb]"
    echo " [--color] [ut]... [-- ut-args]"
    echo "Ex:  export R_HOME=/usr/R.framework/Versions/3.0/Resources"
    echo -n "     ./`basename $0` --branch=master --timeout=600"
    echo " --log=/dev/stdout --debug --valgrind --color ..."
    exit 1
fi
if [ "${1%=*}" = "--timeout" ]; then
    timeout=${1#*=}
    echo "timeout=$timeout"
    shift
fi
if [ "${1%=*}" = "--log" ]; then
    out=/dev/null
    log=${1#*=}
    echo "log=$log"
    shift
fi
if [ "$1" = "--debug" ]; then
    mode=Debug
    modeDir=debug
    echo "debug=1"
    shift
else
    mode=Release
    modeDir=release
fi
if [ "$1" = "--no-clean" ]; then
    noClean=1
    shift
fi
if [ "$1" = "--no-make" ]; then
    noMake=1
    shift
fi
if [ "$1" = "--valgrind" ]; then
    valgrind="valgrind --leak-check=full --show-leak-kinds=all"
    shift
elif [ "$1" = "--gdb" ]; then
    gdb="gdb --args"
    shift
else
    background="&"
fi

# ---------------------------------------------------------------------------- #
# begin
# ---------------------------------------------------------------------------- #
if [ `uname` = "Linux" ]; then
    owner=`stat -c "%U" $0`
elif [ `uname` = "Darwin" ]; then
    owner=`stat -f "%Su" $0`
    ps()
    {
        (unset LD_LIBRARY_PATH DYLD_LIBRARY_PATH; /bin/ps $*)
    }
else
    prog=`cygpath $0`
    owner=`stat -c "%U" $prog`
fi
pdir="/home/$owner/prod"
mkdir -p $pdir/gex-prod-logs/$BRANCH
if [ -z "$log" ]; then
    out=$pdir/gex-prod-logs/$BRANCH/ut.log
    log=$pdir/gex-prod-logs/$BRANCH/make-ut.log
    cat /dev/null >$out
    cat /dev/null >$log
fi

user=`whoami`
if [ "$user" != "$owner" ]; then
    echo "ssh form $user to $owner ..." >$log
    ssh $owner@127.0.0.1 $0 $fullargs
    exit $?
fi

source ~/.gex-prod-conf.sh
if [ `uname` = "Linux" ]; then
    export KERNEL=linux
    export MAKE=make
elif [ `uname` = "Darwin" ]; then
    export KERNEL=mac
    export MAKE=make
else
    export KERNEL=win32
    export MAKE=mingw32-make
    export PATH=$MINGWDIR:/cygdrive/c/icu/dist/lib:$PATH
fi
export MKSPEC=$Makespec
if [ -z "$DEVDIR" ]; then
    export DEVDIR=$pdir/gex-prod-$BRANCH
fi

if [ "$1" = "--color" ]; then
    red="\033[31;1m"
    green="\033[32;1m"
    reset="\033[0m"
    shift
fi

# ---------------------------------------------------------------------------- #
# lib
# ---------------------------------------------------------------------------- #
if [ `uname` = "Linux" ]; then
    if [ `uname -m` = "x86_64" ]; then
        arch=linux64
        arch_chartdir=linux64
        FNP_LIBS=/data/fnp_toolkit_$BRANCH/x64_lsb
        if [ -d /usr/lib64/R ]; then
            R_HOME=/usr/lib64/R
        fi
    else
        arch=linux32
        if [ $BRANCH = "master" ] ||
            [ $BRANCH = "draft" ] ||
            [ ${BRANCH:0:2} = "V7" ]; then
            arch_chartdir=linux32
        else
            arch_chartdir=linux
        fi
        FNP_LIBS=/data/fnp_toolkit_$BRANCH/i86_lsb
    fi
elif [ `uname` = "Darwin" ]; then
    arch=mac
    arch_chartdir=mac
    FNP_LIBS=/home/gexprod/fnp_toolkit_$BRANCH/universal_mac10
else
    if echo $MINGWDIR | grep 64 >/dev/null; then
        arch=win64
        FNP_LIBS=/data/fnp_toolkit_$BRANCH/x64_lsb
    else
        arch=win32
        FNP_LIBS=/data/fnp_toolkit_$BRANCH/i86_lsb
    fi
fi

if [ -z "$R_HOME" ]; then
    export R_HOME=$DEVDIR/other_libraries/R/r_home/$arch
    export R_RLIB=$DEVDIR/other_libraries/R/lib/$arch
else
    export R_RLIB=$R_HOME/lib
fi
export R_LIBS=$R_HOME/library
export GALAXY_R_HOME=$R_HOME

export LD_LIBRARY_PATH="\
$LD_LIBRARY_PATH:\
$QTDIR/lib:\
$FNP_LIBS:\
$R_RLIB:\
$DEVDIR/galaxy_products/gex_product/plugins/lib/$arch:\
$DEVDIR/galaxy_products/gex_product/gex-oc/lib/$arch:\
$DEVDIR/galaxy_products/gex_product/gex-pb/lib/$arch:\
$DEVDIR/galaxy_products/gex_product/gex-log/lib/$arch:\
$DEVDIR/galaxy_libraries/galaxy_qt_libraries/lib/$arch:\
$DEVDIR/galaxy_libraries/galaxy_std_libraries/lib/$arch:\
$DEVDIR/other_libraries/chartdirector/lib/$arch_chartdir:\
$DEVDIR/other_libraries/libqglviewer/lib/$arch:\
$DEVDIR/other_libraries/quazip-0.4.4/lib/$arch:\
$DEVDIR/other_libraries/tufao/0.8/lib/$arch:\
$DEVDIR/other_libraries/pdf_director_thirdparty/lib/$arch:\
$DEVDIR/galaxy_products/gex_product/gex-tester/gtl/lib/$arch\
"
if [ -n "$log" ]; then
    echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH" >>$log
fi

if [ `uname` = "Darwin" ]; then
    export DYLD_LIBRARY_PATH=$LD_LIBRARY_PATH
elif [ `uname -o` = "Cygwin" ]; then
    cat >/tmp/ut.bat <<EOF
set DEVDIR=`cygpath -m $DEVDIR`
set PATH=^
%PATH%;^
`cygpath -m $MINGWDIR`;^
`cygpath -m $QTDIR/bin`;^
`cygpath -m $QTDIR/lib`;^
`cygpath -m $QTDIR/plugins/sqldrivers`;^
`cygpath -m $FNP_LIBS`;^
`cygpath -m $R_RLIB`;^
%DEVDIR%/other_libraries/database/mysql/5.1/$arch;^
%DEVDIR%/galaxy_products/gex_product/plugins/lib/$arch;^
%DEVDIR%/galaxy_products/gex_product/gex-oc/lib/$arch;^
%DEVDIR%/galaxy_products/gex_product/gex-pb/lib/$arch;^
%DEVDIR%/galaxy_products/gex_product/gex-log/lib/$arch;^
%DEVDIR%/galaxy_libraries/galaxy_qt_libraries/lib/$arch;^
%DEVDIR%/galaxy_libraries/galaxy_std_libraries/lib/$arch;^
%DEVDIR%/other_libraries/chartdirector/lib/$arch;^
%DEVDIR%/other_libraries/libqglviewer/lib/$arch;^
%DEVDIR%/other_libraries/quazip-0.4.4/lib/$arch;^
%DEVDIR%/other_libraries/tufao/0.8/lib/$arch;^
%DEVDIR%/other_libraries/qtpropertybrowser-2.5_1-commercial/lib/$arch;^
%DEVDIR%/other_libraries/pdf_director_thirdparty/lib/$arch;^
%DEVDIR%/galaxy_products/gex_product/gex-tester/gtl/lib/$arch;^
C:/icu/dist/lib;^
;
set R_HOME=`cygpath -m $R_HOME`
set R_LIBS=`cygpath -m $R_LIBS`
set GALAXY_R_HOME=`cygpath -m $GALAXY_R_HOME`
%*
EOF
    chmod 755 /tmp/ut.bat
fi

# ---------------------------------------------------------------------------- #
# list / args
# ---------------------------------------------------------------------------- #
if [ -n "$1" ]; then
    list=
    for i in $*; do
        if [ $i = "--" ]; then
            shift
            break
        fi
        list="$list $i"
        shift
    done
else
    list="`cat unit-tests.lst`"
fi
args=$*

# ---------------------------------------------------------------------------- #
# main
# ---------------------------------------------------------------------------- #
cd `dirname $0`
QMAKE=$QTDIR/bin/qmake
if [ $KERNEL != "win32" ]; then
    export DISPLAY=:0
    xhost + >>$log 2>&1
fi
retAll=0
devDir=$DEVDIR
if [ $KERNEL = win32 ]; then
    export DEVDIR=`cygpath -m $DEVDIR`
fi
for f in $list; do
    dir=$devDir/`dirname $f`
    prog=`basename $f`
    if [ -z "$log" ]; then
        printf "$f" | tee -a $out
    fi
    echo "==============================================================" >>$log
    echo "$f" >>$log
    echo "==============================================================" >>$log
    cd $dir >>$log 2>&1
    ret=$?
    if (($ret == 0)); then
        echo "pwd = `pwd`" >>$log
    fi
    if (($ret == 0)) && [ -z "$noMake" ]; then
        if [ -z "$noClean" ]; then
            echo $QMAKE -r -spec $MKSPEC -o Makefile_$KERNEL $prog.pro >>$log
            eval $QMAKE -r -spec $MKSPEC -o Makefile_$KERNEL $prog.pro >>$log\
                2>&1
            ret=$?
            if (($ret == 0)); then
                echo $MAKE -f Makefile_$KERNEL.$mode clean >>$log
                eval $MAKE -f Makefile_$KERNEL.$mode clean >>$log 2>&1
                ret=$?
            fi
        fi
        if (($ret == 0)); then
            echo $MAKE -f Makefile_$KERNEL.$mode >>$log
            eval $MAKE -f Makefile_$KERNEL.$mode >>$log 2>&1
            ret=$?
        fi
    fi
    if (($ret == 0)); then
        if [ $KERNEL = "linux" ]; then
            echo $valgrind $gdb ./$prog $args >>$log
            eval $valgrind $gdb ./$prog $args >>$log 2>&1 $background
        elif [ $KERNEL = "mac" ]; then
            if [ -x $prog ]; then
                if [ -d $prog.app ]; then
                    echo "error: $prog and $prog.app exist" >>$log
                    printf " ut:$red FAIL$reset ($prog and $prog.app exist)\n"\
                        | tee -a $out
                    retAll=$(($retAll + 1))
                    continue
                else
                    echo $valgrind $gdb ./$prog $args >>$log
                    eval $valgrind $gdb ./$prog $args >>$log 2>&1 $background
                fi
            else
                prog=$prog.app/Contents/MacOS/$prog
                echo $valgrind $gdb ./$prog $args >>$log
                eval $valgrind $gdb ./$prog $args >>$log 2>&1 $background
            fi
        else
            full=`readlink -f $modeDir/$prog`
            echo /tmp/ut.bat `cygpath -m $full` $args >>$log
            eval /tmp/ut.bat `cygpath -m $full` $args >>$log 2>&1 &
        fi
        if [ -z "$background" ]; then
            continue
        fi
        pid=$!
        for ((i = 0; $i < $timeout; i++)); do
            if ! ps -p $pid >/dev/null; then
                break
            fi
            sleep 1
        done
        if ps -p $pid >/dev/null; then
            kill -9 $pid
            if [ $KERNEL = "win32" ]; then
                winpath=`cygpath -m $full | sed 's@/@\\\\\\\@g'`
                winpid=`ps -W | grep "$winpath" | awk '{ print $4 }'`
                if [ -n "$winpid" ]; then
                    echo "/bin/kill -f -9 $winpid" >>$log
                    /bin/kill -f -9 $winpid
                fi
            fi
            printf " ut:$red KILL$reset\n" | tee -a $out
            echo "$f: KILL" >>$log
            retAll=$(($retAll + 1))
        else
            wait $pid
            ret=$?
            if (($ret == 0)); then
                printf " ut:$green OK$reset\n" | tee -a $out
                echo "$f: OK" >>$log
            else
                printf " ut:$red FAIL$reset (execution)\n" | tee -a $out
                echo "$f: FAIL (execution)" >>$log
                retAll=$(($retAll + 1))
            fi
        fi
    else
        printf " ut:$red FAIL$reset (compilation)\n" | tee -a $out
        echo "$f: FAIL (compilation)" >>$log
        retAll=$(($retAll + 1))
    fi
done
exit $retAll
