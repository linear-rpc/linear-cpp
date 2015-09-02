#!/bin/sh

result=`./log_stderr_test 2>&1 >/dev/null`

count() {
    word=$1
    echo ${result} | tr [:blank:] "\n" | grep ${word} | wc -l
}

disp=`count DISPLAY`
err=`count ERR`
warn=`count WRN`
inf=`count INF`
dbg=`count DBG`

if [ ${disp} -ne 4 -o ${err} -ne 1 -o ${warn} -ne 1 -o ${inf} -ne 1 -o ${dbg} -ne 1 ]; then
    echo "invalid output"
    echo "disp = ${disp}, hide = ${hide}, err = ${err}, warn = ${warn}, info = ${inf}, dbg = ${dbg}"
    exit 1
fi
exit 0
