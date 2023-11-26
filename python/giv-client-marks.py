#!/usr/bin/python
"""
An example of how to create an external jsonrpc client for controlling
giv.
"""

import requests
import random
import math
import json


for i in range(1024):
  giv_string = ("$marks fcircle\n"
                "$color red\n")
  for j in range(20):
    x = 512 * 1.0*j/20
    phase = 512*1.0*i/20/10
    y = 256+100*math.sin((x+phase) / 512.0 * 2*3.1459)
    giv_string += "%f %f\n"%(x,y)

  # Build a hash and then json.dumps() it
  data = {
    'jsonrpc' : '2.0',
    'method' : 'giv_string',
    'params' : [ giv_string ],
    'id': random.randint(1,100)}
  
  r = requests.post(f'http://localhost:8448', json.dumps(data))
