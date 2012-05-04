#!/usr/bin/env python

import sys
import base64
import json

# Delimeter between form responses
ENTRYDELIM = '----'

# List of objects we expect from the JSON.
# List of attributes and the order in which we want them
# in the output CSV. Values in this list are the names of
# fields in the JSON object. Strings are names of scalar
# fields. Objects in the JSON object are represented as
# maps with one tuple. The key of that tuple is the name
# of the field in the JSON object and the value of that
# tuple is a list of fields in the object. This structure
# is parsed recursively so it can be nested as many times
# as neccessary.
INST_ATTRS = ['name', 'knowledge', 'prep', 'comm', 'comment' ] 
RESP_ATTRS = ['course', 'content', 'labs', 'organization', 'courseComment', 'name', 'sid', {'instructors' : INST_ATTRS }]
ATTRIBUTES = [RESP_ATTRS, INST_ATTRS]

# Maintain a list of instructors here so that the CSV can have all instructors
instructors = set([])

# List of parsed responses
responses = []

def processJSONobject(obj):
  def testfunc(a):
    if type(a) == dict:
      key = a.keys()[0]
      return key in obj
    else:
      return a in obj
  # The JSON object must match one of the predefined objects.
  for attrs in ATTRIBUTES:
    bools = map(lambda a: testfunc(a), attrs);
    if not (False in bools):
      return obj
  print >> sys.stderr, "%s did not match any objects" % (data)
  return None

# Parse a single response
def processLine(s):
  global instructors

  if s == '':
    return

  data = base64.b64decode(s)
  data = json.loads(data, object_hook=processJSONobject)
  responses.append(data)
  for i in data['instructors']:
    instructors.add(i['name'])

# Dump data to CSV
def dumpCSV():
  global instructors
  instructors = sorted(instructors)
  print responses
  print instructors

  # create header row
  hdr = [e for e in RESP_ATTRS if type(e) == str];
  for e in [e for e in RESP_ATTRS if type(e) == dict]:
    key = e.keys()[0]
    name = instructors[i]
    hdr += [name+'-'+s for s in e[key]]*len(instructors)
  print hdr

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

  dumpCSV()

