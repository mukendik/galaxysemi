#!/bin/bash
# ---------------------------------------------------------------------------- #
# common.sh
# ---------------------------------------------------------------------------- #
kernel=`uname`

# ---------------------------------------------------------------------------- #
# sanitycheck
# ---------------------------------------------------------------------------- #
sanitycheck()
{
    # Make sure /usr is mounted
    if [ ! -d /usr/bin ]; then
        echo ""
        echo "[G-INFO] /usr not mounted"
        echo "[G-INFO] Please contact your sysadmin"
        echo ""
        exit 1
    fi

    # Check OS
    if  [ $kernel != "Linux"  ] &&
        [ $kernel != "SunOS"  ] &&
        [ $kernel != "Darwin" ]; then
        echo ""
        echo -n "[G-INFO] Unsupported OS (`uname -sr`) for "
        echo "Galaxy Examinator applications"
        echo "[G-INFO] Make sure you are running Linux"
        echo -n "[G-INFO] Please contact our sales team at "
        echo "sales@galaxysemi.com for further information"
        echo ""
        exit 1
    fi

    # Make sure GEX_PATH env variable is defined
    if [ -z "$GEX_PATH" ]; then
        export GEX_PATH=`pwd -P`
    fi

    # Make sure the executable can be found
    if [ $kernel = "Darwin" ]; then
        app=$GEX_PATH/bin/$1.app
    else
        app=$GEX_PATH/bin/$1
    fi
    if [ ! -x $app ]; then
        echo ""
        echo "[G-INFO] The executable could not be found at :"
        echo "[G-INFO] $app"
        echo ""
        exit 1
    fi
}

# ---------------------------------------------------------------------------- #
# setenvironment
# ---------------------------------------------------------------------------- #
setenvironment()
{
    PATH=/usr/local/bin:/usr/bin:/bin
    PATH=$ORACLE_HOME/bin:$PATH
    export PATH
    export QT_QPA_FONTDIR=$GEX_PATH/lib/fonts/qt
    export QT_QPA_PLATFORM_PLUGIN_PATH=$GEX_PATH/platforms
    LD_LIBRARY_PATH=/usr/lib:/usr/local/lib
    LD_LIBRARY_PATH=$ORACLE_HOME/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=$GEX_PATH/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=$GEX_PATH/fnp_utils:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    if [ $kernel = "Darwin" ]; then
        export DYLD_LIBRARY_PATH=$LD_LIBRARY_PATH
    else
        ldd $GEX_PATH/bin/$1 | grep "not found"
    fi
}

# ---------------------------------------------------------------------------- #
# run
# ---------------------------------------------------------------------------- #
run()
{
    sanitycheck $1

    setenvironment $1

    if [ $kernel = "Darwin" ]; then
        app="$GEX_PATH/bin/$1.app/Contents/MacOS/$1"
    else
        app="$GEX_PATH/bin/$1"
    fi
    shift
    app="$app -style fusion $*"

    if [ -n "$NO_INTERACTIVE" ]; then
        $app &
        pid=$!
        echo "pid=$pid"
        wait $pid
    else
        $app &
    fi

    return $?
}
