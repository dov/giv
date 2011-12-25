#!/usr/bin/python
"""
An example of how to create an external jsonrpc client for controlling
giv.
"""

import httplib
import random

conn = httplib.HTTPConnection('localhost:8222')
headers = {"Content-type": "application/x-www-form-urlencoded",
           "Accept": "text/plain",
           }
example_dir = "/home/dov/github/giv/examples/"
#example_dir = "c:/progra~1/giv/examples"

for i in range(10):
  request = ('{"jsonrpc": "2.0", '
             '"method": "load_file", '
             '"params": [\"'+example_dir+'/%s"], '
             '"id": %d}'%(['lena.pgm','maja.pgm'][i%2],
                          random.randint(1,100)))
  
  conn.request("POST", url="", body= request, headers=headers)
  response = conn.getresponse()
  print response.read()
