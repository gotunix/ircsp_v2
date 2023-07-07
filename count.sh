#!/bin/sh


echo "Total number of HEADER files."
/bin/ls include/*.h | /usr/bin/wc -l

echo "Total number of SOURCE files."
/bin/ls source/*.c | /usr/bin/wc -l

echo "Counting lines of code in source and header files"
/bin/cat include/*.h source/*.c | /usr/bin/wc -l

echo "Counting lines of Comments in Source / Header files"
/bin/cat include/*.h source/*.c | grep // | /usr/bin/wc -l
