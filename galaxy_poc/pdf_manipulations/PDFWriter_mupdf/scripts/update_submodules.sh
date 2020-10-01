#!/bin/bash
project_dir="$1"

# update all sub-modules
echo 'Updating submodules...'

cd "$project_dir"

git submodule init
git submodule update --recursive

cd -

echo 'submodules updated'
