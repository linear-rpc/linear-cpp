#!/usr/bin/env python

from create_header import do_replace
import optparse
import re
import string
import subprocess
import sys

# for creating version.h
GET_COMMIT_ID = 'gite log --pretty=format:%H -1'
VERSION_ID_KEY = '@LINEAR_VERSION_ID@'
COMMIT_ID_KEY = '@LINEAR_COMMIT_ID@'

_usage = 'usage: %prog [options]'


def read_commit_id_and_version(ac, kv):
  try:
    proc = subprocess.Popen(GET_COMMIT_ID,
                            stdout=subprocess.PIPE,
                            shell=True)
    kv[COMMIT_ID_KEY] = proc.stdout.readlines()[0]
  except:
    kv[COMMIT_ID_KEY] = ""
    
  f = open(ac, 'r')
  if f:
    for line in f:
      inner = re.search('AC_INIT\((.*)\)', line)
      if not inner:
        continue
      r = re.findall('\[(.+?)\]', inner.group(1))
      kv[VERSION_ID_KEY] = r[0] + '-' + r[1]

  return kv


if __name__ == '__main__':
    parser = optparse.OptionParser(usage=_usage)
    parser.add_option('-c', '--configure_ac', dest='ac',
                      help='configure.ac file', metavar='/path/to/configure.ac')
    parser.add_option('-i', '--input', dest='input',
                      help='input file', metavar='/path/to/file_name')
    parser.add_option('-o', '--output', dest='output',
                      help='output file', metavar='/path/to/file_name')
    (opts, args) = parser.parse_args()
    if not opts.ac:
        parser.error('you must specify configure.ac file')
    if not opts.input:
        parser.error('you must specify input file')
    if not opts.output:
        parser.error('you must specify output file')

    opts.replace = {
      VERSION_ID_KEY: 'package-version',
      COMMIT_ID_KEY: '-'
    }
    opts.replace = read_commit_id_and_version(opts.ac, opts.replace)
    sys.exit(do_replace(opts))
