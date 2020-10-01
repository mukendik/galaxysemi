TEMPLATE=subdirs
SUBDIRS=quazip

!macx: SUBDIRS+=test/unzip test/zip test/jlcompress test/checksum
