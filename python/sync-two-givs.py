#!/usr/bin/python

######################################################################
#  Example running two instances of giv and syncing
#  transformations between them by copying the transformations
#
#  2023-12-14 Thu
#  Dov Grobgeld <dov.grobgeld@gmail.com>
######################################################################

import requests,json
import argparse
import subprocess
import time
import pdb

parser = argparse.ArgumentParser(description='Process a file')
parser.add_argument('file_a', nargs=1, help='ImageA')
parser.add_argument('file_b', nargs=1, help='ImageB')
args = parser.parse_args()

port_a, port_b = 8458, 8459
for (fn, port) in [(args.file_a[0], port_a),
                   (args.file_b[0], port_b)]:
  subprocess.Popen(['giv', '--port', str(port), fn])

transforms = [None,None]
time.sleep(0.5) # Wait for the process to be created
ports = (port_a, port_b)
while(1):
  new_trans = []
  # Loop over the two children and copy transformations back and forth
  for i in range(2):
    data = {'method':"get_transformation",
            'params':None}
    this_port = ports[i]
    other_port = ports[1-i]

    # On windows we occasionally fail connecting. Make this a no-op
    try:
      r = requests.post(f'http://localhost:{this_port}',
                        json.dumps(data))
    except (ConnectionResetError, requests.exceptions.ConnectionError):
      continue

    root = json.loads(r.text)
    if not 'result' in root:
      continue
    new_transform = tuple(json.loads(r.text)['result'])
    if transforms[i] != new_transform:
      # Copy transformation to other
      data = {'method':"set_transformation",
              'params': new_transform}
      try:
        r = requests.post(f'http://localhost:{other_port}',
                          json.dumps(data))
      except (ConnectionResetError, requests.exceptions.ConnectionError):
        continue

      transforms[i]=  transforms[1-i] = new_transform
      
  time.sleep(0.05)
  
