#!/usr/bin/env python

from create_header import do_replace
import optparse
import os
import platform
import subprocess
import sys

# defs
HAVE_STD_SHARED_PTR_KEY = '@HAVE_STD_SHARED_PTR@'
HAVE_TR1_SHARED_PTR_KEY = '@HAVE_TR1_SHARED_PTR@'
HAVE_BOOST_SHARED_PTR_KEY = '@HAVE_BOOST_SHARED_PTR@'

HAVE_STD_SHARED_PTR = '#define HAVE_STD_SHARED_PTR	(1)'
HAVE_TR1_SHARED_PTR = '#define HAVE_TR1_SHARED_PTR	(1)'
HAVE_BOOST_SHARED_PTR = '#define HAVE_BOOST_SHARED_PTR	(1)'

CXX = os.environ.get('CXX', 'g++')
TRY_COMPILE = '%s -xc++ - -o /dev/null' % CXX
CHECK_STD = """
echo "#include <memory>
int main() {
  std::shared_ptr<int> p;
  return 0;
}" | `%s >/dev/null 2>&1`
echo $?
"""[1:-1] % TRY_COMPILE
CHECK_TR1 = """
echo "#include <tr1/memory>
int main() {
  std::tr1::shared_ptr<int> p;
  return 0;
}" | `%s >/dev/null 2>&1`
echo $?
"""[1:-1] % TRY_COMPILE

_usage = 'usage: %prog [options]'


def check_shared_ptr(kv):
    if sys.platform == 'win32':
        if os.environ.get('GYP_MSVS_VERSION') == '2008':
            kv[HAVE_TR1_SHARED_PTR_KEY] = HAVE_TR1_SHARED_PTR
        else:
            kv[HAVE_STD_SHARED_PTR_KEY] = HAVE_STD_SHARED_PTR
    else:
        proc = subprocess.Popen(CHECK_STD,
                                stdout=subprocess.PIPE,
                                shell=True)
        result = proc.stdout.readlines()[0].decode('utf-8').rstrip('\n\r')
        if result == "0":
            kv[HAVE_STD_SHARED_PTR_KEY] = HAVE_STD_SHARED_PTR
        else:
            proc = subprocess.Popen(CHECK_TR1,
                                    stdout=subprocess.PIPE,
                                    shell=True)
            result = proc.stdout.readlines()[0].rstrip('\n\r')
            if result == "0":
                kv[HAVE_TR1_SHARED_PTR_KEY] = HAVE_TR1_SHARED_PTR
    return kv


if __name__ == '__main__':
    parser = optparse.OptionParser(usage=_usage)
    parser.add_option('-i', '--input', dest='input',
                      help='input file', metavar='/path/to/file_name')
    parser.add_option('-o', '--output', dest='output',
                      help='output file', metavar='/path/to/file_name')
    (opts, args) = parser.parse_args()
    if not opts.input:
        parser.error('you must specify input file')
    if not opts.output:
        parser.error('you must specify output file')

    opts.replace = {
        HAVE_STD_SHARED_PTR_KEY:'#undef HAVE_STD_SHARED_PTR',
        HAVE_TR1_SHARED_PTR_KEY: '#undef HAVE_TR1_SHARED_PTR',
        HAVE_BOOST_SHARED_PTR_KEY: '#undef HAVE_BOOST_SHARED_PTR'
    }
    opts.replace = check_shared_ptr(opts.replace)
    sys.exit(do_replace(opts))
