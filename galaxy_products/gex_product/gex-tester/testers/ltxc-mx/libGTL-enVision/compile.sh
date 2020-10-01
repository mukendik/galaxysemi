# /usr/local/Trolltech/Qt-4.3.0/bin/qmake ~/galaxy_dev/galaxy_libraries/galaxy_std_libraries/gstdl.pro
# make linux_r14_3

/usr/bin/g++ -o libGTLenVision.so.0 -fPIC -shared -DUSE_NS -DSUN_TARGET -DR14_3 -L/usr/lib -lstdc++ -lm -Wl,--whole-archive -lgstdl -lgtl_core -Wl,-no-whole-archive -L../../../../../../galaxy_libraries/galaxy_std_libraries/lib/linux32 -L../../../gtl/lib/linux32 gtl-envision.c -g

