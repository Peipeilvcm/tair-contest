#!/bin/sh

make TARGET_ENGINE=nvm_engine_patentair
# make TARGET_ENGINE=yet_another_nvm_example

cd judge/

./judge.sh ../lib/ 1000000 1000000

cd ..

make clean TARGET_ENGINE=nvm_engine_patentair
# make clean TARGET_ENGINE=yet_another_nvm_example

# rm -rf judge/DB