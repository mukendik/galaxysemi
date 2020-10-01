@echo off
set PATH=c:/mingw/bin;%PATH%
set pocdir=c:/Galaxy/xdr/xdr
set xdrdir=%pocdir%/bsd-xdr-1.0.0
set cflags=-Wall -Werror -fno-strict-aliasing
cd c:\Galaxy\xdr\xdr\bsd-xdr-1.0.0
mkdir mingw
mkdir mingw\lib
gcc -I%xdrdir% %cflags% -o mingw/lib/xdr.o -c lib/xdr.c
gcc -I%xdrdir% %cflags% -o mingw/lib/xdr_array.o -c lib/xdr_array.c
gcc -I%xdrdir% %cflags% -o mingw/lib/xdr_float.o -c lib/xdr_float.c
gcc -I%xdrdir% %cflags% -o mingw/lib/xdr_mem.o -c lib/xdr_mem.c
gcc -I%xdrdir% %cflags% -o mingw/lib/xdr_rec.o -c lib/xdr_rec.c
gcc -I%xdrdir% %cflags% -o mingw/lib/xdr_reference.o -c lib/xdr_reference.c
gcc -I%xdrdir% %cflags% -o mingw/lib/xdr_sizeof.o -c lib/xdr_sizeof.c
gcc -I%xdrdir% %cflags% -o mingw/lib/xdr_stdio.o -c lib/xdr_stdio.c
gcc -I%xdrdir% %cflags% -o mingw/lib/xdr_private.o -c lib/xdr_private.c
gcc -shared -o mingw/mgwxdr-0.dll^
 -Wl,--out-implib=mingw/libxdr.dll.a^
 -Wl,--enable-auto-image-base mingw/xdr.def^
 mingw/lib/xdr.o^
 mingw/lib/xdr_array.o^
 mingw/lib/xdr_float.o^
 mingw/lib/xdr_mem.o^
 mingw/lib/xdr_rec.o^
 mingw/lib/xdr_reference.o^
 mingw/lib/xdr_sizeof.o^
 mingw/lib/xdr_stdio.o^
 mingw/lib/xdr_private.o^
 -lws2_32
ar cr mingw/libxdr.a^
 mingw/lib/xdr.o^
 mingw/lib/xdr_array.o^
 mingw/lib/xdr_float.o^
 mingw/lib/xdr_mem.o^
 mingw/lib/xdr_rec.o^
 mingw/lib/xdr_reference.o^
 mingw/lib/xdr_sizeof.o^
 mingw/lib/xdr_stdio.o^
 mingw/lib/xdr_private.o
cd %homepath%
