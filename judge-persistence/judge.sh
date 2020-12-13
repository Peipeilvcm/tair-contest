JUDGE_LOG=../tasks/judge.log

timeout 90 ./main.sh $1
if [ $? -ne 0 ]; then
    echo "Persistence Test Failed" > ${JUDGE_LOG}
    exit 1
fi
exit 0
