#!/bin/sh

PIN_ROOT=${PWD}/pin-root
cd MyPinTool
export PIN_ROOT=${PIN_ROOT} && make CXX='g++ -std=c++11'
cd ..

cd bin
make TEAM_ID=$1
cd ..

PIN_EXE=${PIN_ROOT}/pin
PINTOOL_SO=./MyPinTool/obj-intel64/MyPinTool.so
DB_NAME=./DB

rm -rf ${DB_NAME}
./bin/warmup ${DB_NAME}

for i in {1..20}
do
    export PMEM_NO_MOVNT=1 && export PMEM_NO_GENERIC_MEMCPY=1 && ${PIN_EXE} -t ${PINTOOL_SO} -r ${i} -- ./bin/onceset ${DB_NAME} ${i}
    ./bin/fulltest ${DB_NAME}
    if [ $? -ne 0 ]; then
        exit 1
    fi
done
exit 0
