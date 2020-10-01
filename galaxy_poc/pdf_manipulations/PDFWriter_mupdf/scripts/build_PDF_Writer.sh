#!/bin/bash

project_dir="$1"
compiler="$2"
build_configuration="$3"
job_count=$4

cd "$project_dir"

cd thirdparty/PDF-Writer

if [ ! -d build/"$build_configuration" ]; then
  mkdir -p build/"$build_configuration"
fi;

cd build/"$build_configuration"

/usr/local/bin/cmake -D CMAKE_CXX_COMPILER="$compiler" -D CMAKE_BUILD_TYPE="$build_configuration" ../..
make -j $job_count
