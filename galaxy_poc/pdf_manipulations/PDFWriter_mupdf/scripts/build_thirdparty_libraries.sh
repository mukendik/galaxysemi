#!/bin/bash

project_dir="$1"
scripts_dir="$project_dir"/scripts
cxx_compiler="$2"
cc_compiler="$3"
build_configuration="$4"
job_count=$5

echo 'Building PDFWriter'
"$scripts_dir"/build_PDF_Writer.sh \
  "$project_dir" "$cxx_compiler"  "$build_configuration" $job_count
echo 'PDFWriter built'

echo 'Building mupdf'
"$scripts_dir"/build_mupdf.sh \
  "$project_dir" "$cxx_compiler" "$cc_compiler" "$build_configuration" \
  $job_count
echo 'mupdf built'
