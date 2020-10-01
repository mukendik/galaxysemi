#!/bin/bash

project_dir="$1"
qmake_bin_dir="$2"
build_configuration="$3"

cd "$project_dir"

echo 'Updating submodules...'

git submodule init
git submodule update --recursive

echo 'submodules updated'

cd thirdparty/KDReports

# modifying the PATH var to include the qt bin directory
PATH="$qmake_bin_dir":$PATH

echo 'configure KDReport build...'

# make sure all is clean
make distclean 2>&1

# redirecting stderr to stdout as msg in stderr are not errors
./autogen.py 2>&1
./configure.sh -"$build_configuration" -static -no-unittests 2>&1

echo 'configure KDReport build complete'
