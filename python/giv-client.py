#!/usr/bin/python
######################################################################
#  A minimal json client
#
#  2023-11-26 Sun
#  Dov Grobgeld <dov.grobgeld@gmail.com>
######################################################################
import requests,json

data = {'method':"load_file",
        'params':['/home/dov/github/giv/examples/maja.pgm']} # Full path!
r = requests.post(f'http://localhost:8448', json.dumps(data))

