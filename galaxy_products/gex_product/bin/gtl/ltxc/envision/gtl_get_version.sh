#!/bin/bash
ln -sf libGTLenVision.so.0 libGTLenVision.so
g++ -o gtl_get_version gtl_get_version.c -lrt -lpthread -ldl -L. -lGTLenVision
LD_LIBRARY_PATH=. ./gtl_get_version
