#!/usr/bin/env python

import sys
import base64
import urlparse
import random

# Delimeter between form responses
ENTRYDELIM = '----'

# List of parsed responses
course_responses = []
inst_responses = []

# Parse a single response
def processLine(s):
  global course_responses, inst_responses

  if s == '':
    return

  data = base64.b64decode(s)
  data = urlparse.parse_qs(data)

  getData = lambda e: data[e][0] if e in data.keys() else None

  course = dict()
  course['id'] = random.randint(1, 1000000)
  course['name'] = getData('course-name')
  course['content'] = getData('course-content')
  course['labs'] = getData('course-labs')
  course['org'] = getData('course-org')
  course['comment'] = getData('course-comment')
  course['respondent'] = getData('name')
  course['respond-sid'] = getData('sid')
  course_responses.append(course)

  for i in range(0,20): #only support up to 20 instructors...
    inst = dict()
    inst['respid'] = course['id']
    inst['name'] = getData('inst-name-%d'%(i))
    inst['know'] = getData('inst-know-%d'%(i))
    inst['prep'] = getData('inst-prep-%d'%(i))
    inst['comm'] = getData('inst-comm-%d'%(i))
    inst['comments'] = getData('inst-comment-%d'%(i))

    if inst['name'] == None:
      break
    inst_responses.append(inst)

# Dump data to CSV
def dumpCSV():
  print course_responses
  print inst_responses

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

