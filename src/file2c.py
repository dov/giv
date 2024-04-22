#!/usr/bin/env python3
######################################################################
#  Wrap an arbitrary file for inclusion in c.
#
#  2024-04-21 Sun
#  Dov Grobgeld <dov.grobgeld@gmail.com>
######################################################################

import argparse
import re

def file2c(target, source):
    out = open(target, "wb")
    inp = open(source, "rb")

    for line in inp.readlines():
        line = line.decode('utf8').rstrip()
        line = re.sub("\\\\", "\\\\", line)
        line = re.sub("\\\"", "\\\"", line)
        line = '"'+line+'\\n"\n'
        out.write(line.encode('utf8'))
        
    out.close()
    inp.close()

parser = argparse.ArgumentParser(description='Process a file')
parser.add_argument('--target',
                    dest='target',
                    action='store',
                    type=str,
                    default=None,
                    help='target filename')
parser.add_argument('--source',
                    dest='source',
                    action='store',
                    default=None,
                    help='source filename editing')

args = vars(parser.parse_args())
print(f'{args["target"]=} {args["source"]=}')
if not args['target'] or not args['source']:
  sys.write('Need both target asnd source arguments!')

file2c(args['target'],
       args['source'])
