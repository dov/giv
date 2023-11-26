#!/usr/bin/python
"""
An example of how to create an external jsonrpc client for controlling
giv.
"""

import httplib
import random
import math

conn = httplib.HTTPConnection('localhost:8448')
headers = {"Content-type": "application/x-www-form-urlencoded",
           "Accept": "text/plain",
           }

for i in range(1024):
  giv_string = ("$marks fcircle\n"
                "$color red\n")
  for j in range(20):
    x = 512 * 1.0*j/20
    phase = 512*1.0*i/20/10
    y = 256+100*math.sin((x+phase) / 512.0 * 2*3.1459)
    giv_string += "%f %f\n"%(x,y)

  request = ('{"jsonrpc": "2.0", '
             '"method": "giv_string", '
             '"params": [\"'+giv_string+'"], '
             '"id": %d}'%(random.randint(1,100)))
  
  conn.request("POST", url="", body= request, headers=headers)
  response = conn.getresponse()
  print response.read()
