#!/bin/bash
# ---------------------------------------------------------------------------- #
# pssh.sh
# ---------------------------------------------------------------------------- #
if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    echo "Usage: `basename $0` [ssh-options]"
    echo "Ex:    REMOTE=1 $0"
    echo "       $0 -Y"
    echo "       $0 -o ServerAliveInterval=30"
    exit 0
fi

echo " (1) http"
echo " (2) vnc"
echo " (3) mysql"
echo " (4) ftp(beta)"
#echo " (5) rdp"
echo -n " [1] "
read ret
case "$ret" in
    2) local_port=5900; remote_port=5900 ;;
    3) local_port=3306; remote_port=3306 ;;
    4) local_port=21  ; remote_port=21   ;;
#    5) local_port=3889; remote_port=3889 ;;
    *) remote_port=80 ;;
esac
if [ -n "$local_port" ]; then
    echo -n "remote port ? [$remote_port] "
    read ret
    if [ -n "$ret" ]; then
        remote_port=$ret
    fi
    echo -n " local port ? [$local_port] "
    read ret
    if [ -n "$ret" ]; then
        local_port=$ret
    fi
fi

kernel=`uname`
ssh_options="$*"

# ---------------------------------------------------------------------------- #
# setName
# ---------------------------------------------------------------------------- #
setName()
{
    case $1 in
        1) name="dunharrow-prod"    ;;
        2) name="durthang-prod"     ;;
        3) name="barad-nimras-prod" ;;
        4) name="barad-eithel-prod" ;;
        5) name="lugburz-prod"      ;;
        6) name="dol-guldur-prod"   ;;
        7) name="isengard-prod"     ;;
        8) name="cirith-ungol-prod" ;;
        9) name="callisto"          ;;
       10) name="saturne"           ;;
       11) name="pluton"            ;;
       12) name="sisense"           ;;
       13) name="dagorlad-prod"     ;;
       14) name="numenor-prod"      ;;
       15) name="mirkwood-prod"     ;;
       16) name="fornost-prod"      ;;
       17) name="windows5neptune"   ;;
       18) name="gfactory"          ;;
        *) name= ; return 1 ;;
    esac
    return 0
}

# ---------------------------------------------------------------------------- #
# setDesc
# ---------------------------------------------------------------------------- #
setDesc()
{
    case $1 in
        1) desc="jupiter  ubuntu  64 LG3" ;;
        2) desc="jupiter  ubuntu     LG2" ;;
        3) desc="neptune  fedora v66 LG1" ;;
        4) desc="pluton   centos v4x LG1" ;;
        5) desc="neptune  windows 64 WG2" ;;
        6) desc="neptune  windows    WG1" ;;
        7) desc="venus    solaris    SG1" ;;
        8) desc="mac      darwin  64 MG1" ;;
        9) desc="callisto ubuntu        " ;;
       10) desc="saturne  windows       " ;;
       11) desc="pluton   centos v7x    " ;;
       12) desc="sisense  Win 7 64      " ;;
       13) desc="centos58-64 centos 5.8 64bits" ;;
       14) desc="centos58-32 centos 5.8 32bits" ;;
       15) desc="windows7-64 windows 64" ;;
       16) desc="windows7-32 windows 32" ;;
       17) desc="windows-XP-32 windows 32" ;;
       18) desc="gfactory debian testing 64bits";;
        *) desc= ; return 1 ;;
    esac
    return 0
}

# ---------------------------------------------------------------------------- #
# setAddr
# ---------------------------------------------------------------------------- #
setAddr()
{
    case $1 in
        1) n=77 ;;
        2) n=74 ;;
        3) n=44 ;;
        4) n=65 ;;
        5) n=80 ;;
        6) n=52 ;;
        7) n=67 ;;
        8) n=46 ;;
        9) n=40 ;;
       10) n=25 ;;
       11) n=54 ;;
       12) n=55 ;;
       13) n=39 ;;
       14) n=31 ;;
       15) n=98 ;;
       16) n=55 ;;
       17) n=52 ;;
       18) n=228 ;;
        *) addr= ; return 1 ;;
    esac
    if [ $remote = 0 ]; then
        addr="192.168.1.$n"
    else
        # I remember these IP from Dourdan's box in case the dyn dns is broken... 
        #addr="93.0.100.217 -p ${n}22"
        #addr="79.87.44.166 -p ${n}22"
        # This one works may 7th 2014
        #addr="62.62.172.191 -p ${n}22"
        addr="dourdan.galaxysemi.com -p ${n}22"
    fi
    return 0
}

# ---------------------------------------------------------------------------- #
# setPort
# ---------------------------------------------------------------------------- #
setPort()
{
    if [ -z "$local_port" ]; then
        port=$((8080 + $1 - 1))
    else
        port=$local_port
    fi
    return 0
}

# ---------------------------------------------------------------------------- #
# setSsh
# ---------------------------------------------------------------------------- #
setSsh()
{
    ssh=
    user=gexprod
    setName $1 || return
    if [ $name = callisto ]; then
        user=licman
    fi
    if [ -n "$remote_port" ]; then
        setAddr $1 &&\
        setPort $1 &&\
        ssh="ssh $user@$addr -L $port:$name:$remote_port $ssh_options"
    else
        setAddr $1 &&\
        ssh="ssh $user@$addr $ssh_options"
    fi
}

# ---------------------------------------------------------------------------- #
# printUrl
# ---------------------------------------------------------------------------- #
printUrl()
{
    if [ "$remote_port" = 80 ]; then
        setName $1 &&\
        setPort $1 &&\
        echo "http://$name:$port"
    elif [ "$local_port" = 21 ]; then
        echo "ftp://$name:$port"
    elif [ "$local_port" = 5900 ]; then
        if [ $kernel = "Linux" ]; then
            echo "vncviewer localhost"
        else
            echo "vnc localhost"
        fi
    fi
}

openUrl()
{
    if [ $kernel = "Linux" ]; then
        echo "Any command to open a browser/url on linux ?"
    else
        # Opening the registered browser
        # timeout 3 
        # Is there any command to run a command after few seconds to be sure the connection is established ?
        START "" "http://$name:$port"
    fi
}

# ---------------------------------------------------------------------------- #
# main
# ---------------------------------------------------------------------------- #
echo
i=1
while setName $i; do
    setDesc $i
    j=`printf "%2d" $i`
    echo "$j) $desc $name"
    i=$(($i + 1))
done
nb_s=$(($i - 1))
echo -n " ? "
if [ -z "$REMOTE" ]; then
    remote=0
    setAddr 1
    if [ $kernel = "Linux" ]; then
        ping -c 1 $addr >/dev/null 2>&1
    else
        ping -n 1 $addr >/dev/null 2>&1
    fi
    remote=$?
else
    remote=$REMOTE
fi
read server

if [ -n "$server" ] && echo "$server" | grep " "; then
    if ! which xterm >/dev/null; then
        echo "xterm is needed"
        exit 1
    fi
    if ! which screen >/dev/null; then
        echo "screen is needed"
        exit 1
    fi

    if ! grep "%-w" ~/.screenrc >/dev/null 2>&1; then
        echo
        echo "todo in ~/.screenrc :"
        echo
        hostname=`hostname -s`
        if [ -z "$hostname" ]; then
            hostname=`hostname`
            echo
        fi
        cat << EOF
altscreen on
bindkey ^[[1;2D prev
bindkey ^[[1;2C next
hardstatus alwayslastline "%{= dg} %=%-w%{b ..}%n %t%{= ..}%+w"
defscrollback 5000
msgwait 1
# C-a e
bind e stuff 'printf "\ek$hostname\e\\\\"'
EOF
        echo
    fi

    echo
    pass=0
    for i in $server; do
        setSsh $i
        if [ $pass = 0 ]; then
            xterm -geometry 120 -title pssh -e "screen -m -S pssh $ssh" &
            printUrl $i
            pass=1
        else
            if screen -S pssh -X screen $ssh; then
                printUrl $i
            fi
        fi
        sleep 1
    done
    echo
    echo "press enter to quit"
    read ret
    screen -S pssh -X quit
else
    if [ -z "$server" ] || (($server < 1)); then
        exit 0
    fi

    echo
    printUrl $server
    echo

    setSsh $server
    echo $ssh
    if [ "$remote_port" = 80 ]; then
        openUrl
    fi
    if [ "$local_port" = 5900 ]; then
        echo runing vncviewer...
        vncviewer &
    fi
    eval $ssh
    if [ $? != 0 ]; then 
        read k
    fi
fi

exit $?
