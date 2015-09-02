#!/bin/sh

result=`./log_macro4function_test 2>&1 >/dev/null`

count() {
    word=$1
    echo ${result} | tr [:blank:] "\n" | grep ${word} | wc -l
}

disp=`count DISPLAY`
hide=`count HIDE`
err=`count ERROR`
warn=`count WARNING`
inf=`count INFO`
dbg=`count DEBUG`

if [ ${disp} -ne 2 -o ${hide} -ne 0 -o ${err} -ne 1 -o ${warn} -ne 1 -o ${inf} -ne 0 -o ${dbg} -ne 0 ]; then
    echo "invalid output"
    echo "disp = ${disp}, hide = ${hide}, err = ${err}, warn = ${warn}, info = ${inf}, dbg = ${dbg}"
    exit 1
fi
exit 0
