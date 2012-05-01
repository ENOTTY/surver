#!/usr/bin/env python

import sys
import base64

# Delimeter between form responses
ENTRYDELIM = '----'

# Delimeter between fields in the POST string
FIELDDELIM = '&'

# List of attributes we want to get in the POST string
ATTRIBUTES = ['course', 'cont', 'labs', 'org', 'course-comment', 'name', 'sid']

# List of attributes we want to see in the output CSV
CSVHEADER = ATTRIBUTES + ['instructors']

# Maintain a list of instructors here so that the CSV can have all instructors
instructors = []

# List of parsed responses
entries = []

# Parse a single response
def processLine(s):
  global instructors

  if s == '':
    return

  data = base64.b64decode(s)
  data = data.split('&')
  data = map(lambda x: x.split('=', 1), data)
  data = filter(lambda x: len(x) == 2, data)

  # Right now we do a very not robust thing where if the attribute ends in a
  # digit, we assume that it's data for an instructor.
  entry = dict()
  for (attr, val) in data:
    if attr[0:9] == 'instname-':
      if val not in instructors:
        instructors += [val]
      if 'instructors' not in entry.keys():
        entry['instructors'] = dict()
      entry['instructors'][val] = dict()
    elif attr in ATTRIBUTES:
      entry[attr] = val
    else:
      print "Unknown attribute found: %s" % (attr)

  entries.append(entry)

  print data

# Dump data to CSV
def dumpCSV():
  print entries

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

  print instructors
  dumpCSV()

