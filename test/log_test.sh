#!/bin/sh

result=`./log_test 2>&1 >/dev/null`

count() {
    word=$1
    echo ${result} | tr [:blank:] "\n" | grep ${word} | wc -l
}

disp=`count DISPLAY`
hide=`count HIDE`
sys=`count SYS`
err=`count ERR`
warn=`count WRN`
if [ ${hide} -ne 0 -o ${sys} -ne 0 -o ${err} -ne 0 -o ${warn} -ne 0 ]; then
    echo "invalid output"
    exit 1
fi
exit 0
