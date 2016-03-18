#!/usr/bin/env python

from ast import literal_eval
import optparse
import re
import sys

_usage = 'usage: %prog [options]'


def do_replace(opts):
    if not opts.input or not opts.output or not opts.replace:
        return -1
    f = open(opts.input, 'r')
    if not f:
        return -1
    inp = f.read()
    rx = re.compile('|'.join(map(re.escape, opts.replace)))
    def one_xlat(match):
        return opts.replace[match.group(0)]
    outp = rx.sub(one_xlat, inp)
    open(opts.output, 'w').write(outp)
    return 0


if __name__ == '__main__':
    parser = optparse.OptionParser(usage=_usage)
    parser.add_option('-i', '--input', dest='input',
                      help='input file', metavar='/path/to/file_name')
    parser.add_option('-o', '--output', dest='output',
                      help='output file', metavar='/path/to/file_name')
    parser.add_option('-r', '--replace', dest='replace',
                      help='replace template: k->v', metavar='\"{\'k1\':v1, \'k2\':v2}\"')
    (opts, args) = parser.parse_args()
    if not opts.input:
        parser.error('you must specify input file')
    if not opts.output:
        parser.error('you must specify output file')
    try:
        opts.replace = literal_eval(opts.replace) if opts.replace else {}
    except:
        parser.error('you must specify valid keyvalue pair')
    sys.exit(do_replace(opts))
