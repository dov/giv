#!/usr/bin/python

import httplib,json

conn = httplib.HTTPConnection('localhost:8448')

request = {'method':"load_file",
           'params':['/home/dov/github/giv/examples/lena.pgm']} # Full path!

conn.request("POST", url='', body= json.dumps(request))
response = json.loads(conn.getresponse().read())['result']
print response
