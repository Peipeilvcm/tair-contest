#!/bin/bash

INCLUDE_DIR="../include"
LIB_PATH=../lib/

g++ -o test-setandget test-setandget.cpp\
	-L $LIB_PATH -l engine \
	-I $INCLUDE_DIR \
    -g \
    -std=c++0x \
    -lpmem


if [ $? -ne 0 ]; then
    echo "Compile Error"
    exit 7 
fi

DB_NAME=./DB
# ./warmup ${DB_NAME}

./test-setandget ${DB_NAME}

rm ./test-setandget

rm -rf DB*
