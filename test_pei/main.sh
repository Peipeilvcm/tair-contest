#!/bin/bash

INCLUDE_DIR="../include"
LIB_PATH=../lib/


g++ -pthread -o warmup warmup.cpp\
	-L $LIB_PATH -l engine \
	-I $INCLUDE_DIR \
    -g \
    -mavx2 \
    -std=c++11 \
    -lpmem

g++ -pthread -o onceset onceset.cpp\
	-L $LIB_PATH -l engine \
	-I $INCLUDE_DIR \
    -g \
    -mavx2 \
    -std=c++11 \
    -lpmem

g++ -pthread -o fulltest fulltest.cpp\
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

for i in {1..20}
do
    ./onceset ${DB_NAME} ${i}
    ./fulltest ${DB_NAME}
    if [ $? -ne 0 ]; then
        exit 1
    fi
done

