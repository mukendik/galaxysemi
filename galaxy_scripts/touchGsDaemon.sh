#!/bin/bash
if [ `basename $(pwd)` = "galaxy_scripts" ]; then
    cd ..
fi
find . -name "*.cpp" -exec grep -q GSDAEMON {} \; -exec touch {} \; -print
find . -name "*.h"   -exec grep -q GSDAEMON {} \; -exec touch {} \; -print
