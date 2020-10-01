#!/bin/sh
set -xe
export TARGET="./target"
../../junit.sh \
  --debug \
  --dryrun \
  --pwd=$TARGET \
  --output=$TARGET/result.xml \
  --package=package \
  --classname=classname \
  --testname=testname \
  true
[ $? -eq 0 ]

../../junit.sh \
  --debug \
  --dryrun \
  --pwd=$TARGET \
  --output=$TARGET/result.xml \
  --package=package \
  --classname=classname \
  --testname=testname \
  false
[ $? -eq 0 ]
