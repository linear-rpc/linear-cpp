#!/bin/sh

result=`./log_onoff_test 2>&1 >/dev/null`

count() {
    word=$1
    echo ${result} | tr [:blank:] "\n" | grep ${word} | wc -l
}

disp=`count DISPLAY`
hide=`count HIDE`
err=`count ERR`
warn=`count WRN`
inf=`count INF`
dbg=`count DBG`
if [ ${disp} -ne 6 -o ${hide} -ne 0 -o ${err} -ne 2 -o ${warn} -ne 2 -o ${inf} -ne 1 -o ${dbg} -ne 1 ]; then
    echo "invalid output"
    echo "hide = ${hide}, disp = ${disp}, err = ${err}, warn = ${warn}, info = ${inf}, dbg = ${dbg}"
    exit 1
fi
exit 0
