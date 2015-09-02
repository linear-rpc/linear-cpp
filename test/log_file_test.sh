#!/bin/sh

test_log_file="./test.log"
rm -f ${test_log_file}
./log_file_test

count() {
    word=$1
    cat ${test_log_file} | tr [:blank:] "\n" | grep ${word} | wc -l
}

disp=`count DISPLAY`
hide=`count HIDE`
err=`count ERR`
warn=`count WRN`
inf=`count INF`
dbg=`count DBG`

if [ ${disp} -ne 4 -o ${hide} -ne 0 -o ${err} -ne 1 -o ${warn} -ne 1 -o ${inf} -ne 1 -o ${dbg} -ne 1 ]; then
    echo "invalid output"
    echo "disp = ${disp}, hide = ${hide}, err = ${err}, warn = ${warn}, info = ${inf}, dbg = ${dbg}"
    exit 1
fi
rm -f ${test_log_file}
exit 0
