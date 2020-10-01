#/bin/bash

project_dir="$1"
build_configuration="$2"
job_count=$3

cd "$project_dir"
cd thirdparty/KDReports

echo 'Building KDReports ( '"$build_configuration"' )'
make -j $job_count
echo 'KDReport build terminated'
