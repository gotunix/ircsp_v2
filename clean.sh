#!/bin/sh

make clean > /dev/null
rm -rf clean
rm -rf aclocal.m4 config.guess config.sub depcomp libtool missing 
rm -rf config.h  configure ltmain.sh config.h.in Makefile autom4te.cache
rm -rf config.log INSTALL COPYING config.status install-sh Makefile.in 
rm -rf stamp-h1 source/*.o source/Makefile source/Makefile.in 
rm -rf source/.deps source/ircsp
rm -rf configure.output
