#!/usr/bin/env python

import os
import platform
import re
import string
import subprocess
import sys

# for creating version.h
GET_COMMIT_ID = 'git log --pretty=format:%H -1'
VERSION_ID_KEY = '@LINEAR_VERSION_ID@'
COMMIT_ID_KEY = '@LINEAR_COMMIT_ID@'

def read_commit_id(values):
  proc = subprocess.Popen(GET_COMMIT_ID,
                          stdout=subprocess.PIPE,
                          shell=True)
  values[COMMIT_ID_KEY] = proc.stdout.readlines()[0]


def read_version_from_configure_ac(values):
  f = open('configure.ac', 'r')
  for line in f:
    inner = re.search('AC_INIT\((.*)\)', line)
    if not inner:
      continue
    r = re.findall('\[(.+?)\]', inner.group(1))
    values[VERSION_ID_KEY] = r[0] + '-' + r[1]

def process():
  values = dict()
  read_commit_id(values)
  read_version_from_configure_ac(values)

  inp = open('include/linear/version.h.in', 'r').read()
  outp = inp.replace(VERSION_ID_KEY, values[VERSION_ID_KEY]).replace(COMMIT_ID_KEY, values[COMMIT_ID_KEY])
  open('include/linear/version.h', 'w').write(outp)

if __name__ == '__main__':
    sys.exit(process())
