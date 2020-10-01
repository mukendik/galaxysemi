#!/bin/bash

project_dir="$1"
cxx_compiler="$2"
cc_compiler="$3"
build_configuration="$4"
job_count=$5

cd "$project_dir"

cd thirdparty/mupdf

make -j $job_count build=$build_configuration \
  HAVE_X11=no \
  CC=$cc_compiler \
  CXX=$cxx_compiler
