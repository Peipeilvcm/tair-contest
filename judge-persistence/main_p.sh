#!/bin/sh
cd bin
make TEAM_ID=$1
cd ..

DB_NAME=/DB

rm -rf ${DB_NAME}
./bin/warmup ${DB_NAME} > /dev/null 2>&1

