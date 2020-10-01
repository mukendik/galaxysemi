#!/bin/bash
# ---------------------------------------------------------------------------- #
# install_ym_daemon.sh
# ---------------------------------------------------------------------------- #
service=QX-Yield-Man

./gs-ymd -i

regkey=/HKLM/SYSTEM/CurrentControlSet/services/$service/ImagePath
if ! regtool get $regkey >/dev/null; then
    echo "warning: $regkey not found"
    regkey=/HKLM/SYSTEM/ControlSet002/services/$service/ImagePath
fi
if ! regtool get $regkey >/dev/null; then
    echo "warning: $regkey not found"
    regkey=/HKLM/SYSTEM/ControlSet001/services/$service/ImagePath
fi
if regtool get $regkey >/dev/null; then
    imagepath=`regtool get $regkey`
    if ! echo "$imagepath" | grep " \-platform minimal"; then
        regtool -s set $regkey "$imagepath -platform minimal"
    fi
else
    echo "error: $regkey not found"
fi
