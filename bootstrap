#!/bin/sh

rm -rf aclocal.m4 autom4te.cache build-aux m4/libtool.m4 m4/lt* \
        Makefile.in src\Makefile.in configure config.* Makefile \
        src/Makefile src/.deps libtool src/*.o src/.libs src/*.exe

mkdir build-aux

aclocal -I m4
libtoolize --copy
automake --add-missing --copy
autoconf
