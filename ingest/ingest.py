#!/usr/bin/env python

import sys
import base64

ENTRYDELIM = '----'
FIELDDELIM = '&'

def processLine(str):
  if str == '':
    return
  data = base64.b64decode(str)

if __name__ == "__main__":

  if len(sys.argv) < 2:
    sys.exit('Usage: %s <datafile>' % (sys.argv[0]))

  DATAFILE = sys.argv[1]

  f = open(DATAFILE, 'r')

  curstr = ''

  while True:
    line = f.readline()
    if line == '':
      break

    line = line.strip()

    if line == ENTRYDELIM:
      processLine(curstr)
      curstr = ''
    else:
      curstr += line


  f.close()

