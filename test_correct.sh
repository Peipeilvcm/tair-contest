#!/bin/sh

make TARGET_ENGINE=nvm_engine_latest
# make TARGET_ENGINE=yet_another_nvm_example

cd test_pei/

# ./main_correctmy.sh
./main_correct.sh


cd ..

make clean TARGET_ENGINE=nvm_engine_latest
# make clean TARGET_ENGINE=yet_another_nvm_example

# rm -rf judge/DB