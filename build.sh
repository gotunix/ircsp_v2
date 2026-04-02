#!/bin/sh

echo "Running [clean.sh]"
/bin/sh clean.sh

echo "Running [autogen.sh]"
/bin/sh autogen.sh

echo "Building..."
make clean > /dev/null
make
echo "Build completed..."
