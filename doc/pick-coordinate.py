#!/usr/bin/python
import httplib,json

conn = httplib.HTTPConnection('localhost:8222')

request = {'method':"pick_coordinate",
           'params':[]}

conn.request("POST", url='', body= json.dumps(request))
response = json.loads(conn.getresponse().read())
if 'error' in response:
  print response['error']['message']
else:
  print response['result']

