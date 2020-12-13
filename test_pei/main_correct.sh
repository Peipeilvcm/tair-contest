#!/bin/bash

INCLUDE_DIR="../include"
LIB_PATH=../lib/

g++ -pthread -o test-set test-set.cpp\
	-L $LIB_PATH -l engine \
	-I $INCLUDE_DIR \
    -g \
    -mavx2 \
    -std=c++11 \
    -lpmem

g++ -pthread -o test-get test-get.cpp\
	-L $LIB_PATH -l engine \
	-I $INCLUDE_DIR \
    -g \
    -mavx2 \
    -std=c++11 \
    -lpmem

if [ $? -ne 0 ]; then
    echo "Compile Error"
    exit 7 
fi

DB_NAME=./DB
# ./warmup ${DB_NAME}

./test-set ${DB_NAME}
./test-get ${DB_NAME}


