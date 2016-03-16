#!/usr/bin/env python

import os
import platform
import re
import string
import subprocess
import sys

# for creating memory.h
adict = {
    '@HAVE_STD_SHARED_PTR@':'#undef HAVE_STD_SHARED_PTR',
    '@HAVE_TR1_SHARED_PTR@': '#undef HAVE_TR1_SHARED_PTR',
    '@HAVE_BOOST_SHARED_PTR@': '#undef HAVE_BOOST_SHARED_PTR'
}
HAVE_STD_SHARED_PTR = '#define HAVE_STD_SHARED_PTR 1'
HAVE_TR1_SHARED_PTR = '#define HAVE_TR1_SHARED_PTR 1'
HAVE_BOOST_SHARED_PTR = '#define HAVE_BOOST_SHARED_PTR 1'

CXX = os.environ.get('CXX', 'g++')
TRY_COMPILE = '%s -xc++ - -E' % CXX
CHECK_TR1 = """
echo "#include <tr1/memory>
int main() {
  std::tr1::shared_ptr<int> p;
  return 0;
}" | `%s >/dev/null 2>&1`
echo $?
"""[1:-1] % TRY_COMPILE
CHECK_STD = """
echo "#include <memory>
int main() {
  std::shared_ptr<int> p;
  return 0;
}" | `%s >/dev/null 2>&1`
echo $?
"""[1:-1] % TRY_COMPILE

def check_shared_ptr():
    if sys.platform == 'win32':
        if os.environ.get('GYP_MSVS_VERSION') == '2008':
            adict['@HAVE_TR1_SHARED_PTR@'] = HAVE_TR1_SHARED_PTR
        else:
            adict['@HAVE_STD_SHARED_PTR@'] = HAVE_STD_SHARED_PTR
    else:
        proc = subprocess.Popen(CHECK_TR1,
                                stdout=subprocess.PIPE,
                                shell=True)
        result = proc.stdout.readlines()[0].rstrip('\n\r')
        if result == "0":
            adict['@HAVE_TR1_SHARED_PTR@'] = HAVE_TR1_SHARED_PTR
        else:
            proc = subprocess.Popen(CHECK_STD,
                                    stdout=subprocess.PIPE,
                                    shell=True)
            result = proc.stdout.readlines()[0].rstrip('\n\r')
            if result == "0":
                adict['@HAVE_STD_SHARED_PTR@'] = HAVE_STD_SHARED_PTR

def process():
    check_shared_ptr()
    inp = open('include/linear/memory.h.in', 'r').read()
    rx = re.compile('|'.join(map(re.escape, adict)))
    def one_xlat(match):
        return adict[match.group(0)]
    outp = rx.sub(one_xlat, inp)
    open('include/linear/memory.h', 'w').write(outp)

if __name__ == '__main__':
    sys.exit(process())
