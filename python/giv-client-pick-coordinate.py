#!/usr/bin/python

######################################################################
#  An example of how to pick a coordinate with giv.
#
#  2023-11-26 Sun
#  Dov Grobgeld <dov.grobgeld@gmail.com>
######################################################################

import requests,json

def get_giv_coordinate(port=8448):
  '''Returns x,y,button,modifiers'''
  data = {'method':"pick_coordinate",
          'params':[]} 
  
  r = requests.post(f'http://localhost:{port}', json.dumps(data))
  res = json.loads(r.text)
  return res['result']

x,y,button,modifiers = get_giv_coordinate()
print(f'{x=} {y=} {button=} {modifiers=}')
  
