#!/usr/bin/env python

# TODO only support text files

import BaseHTTPServer
from Cookie import SimpleCookie
import random
import string
import os.path
import ssl
import sys

COOKIE_LENGTH = 52
SESSION_ID = 'sess-id'
HOMEPAGE = 'survey.html'
DELIMITER = '?'
USESSL = False
CERT = ''
KEYFILE = ''

class Client:

  """
  more is a dictionary for additional information. It should be managed
  by the user of this class.
  """
  def __init__(self, ip, mac, cookie, useragent, more):
    self.ip = ip
    self.mac = mac
    self.cooke = cookie
    self.ua = useragent
    self.attrMap = more

class MyHandler(BaseHTTPServer.BaseHTTPRequestHandler):
  def send(self, path):
    print >> sys.stderr, ''
    print >> sys.stderr, self.headers

    text = open(path).read()

    self.send_response(200)

    # handle file
    self.send_header('Content-length', len(text))

    # handle cookies
    cookie = None
    if 'cookie' in self.headers.keys():
      cookies = SimpleCookie(self.headers['cookie'])
      cookie = cookies[SESSION_ID].value
    if cookie == None:
      cookie = ''.join(random.choice(string.ascii_uppercase + string.ascii_lowercase + string.digits) for x in range(COOKIE_LENGTH))
      self.send_header('Set-Cookie', '%s=%s' % (SESSION_ID, cookie))

    self.end_headers()

    # send the file
    self.wfile.write(text)

  def do_GET(self):
    # handle a request without a path
    if self.path == '/':
      self.send(os.path.abspath(HOMEPAGE))
      return

    # make sure requested path is under working directory
    desiredPath = os.path.abspath(".%s" % (self.path))
    if os.getcwd() in desiredPath and os.path.exists(desiredPath):
      self.send(desiredPath)

    # file does not exist
    else:
      self.send_response(404)

  """Print data to stdout"""
  def parsePostdata(self, data, prefix=[], postfix=[]):
    data = filter(lambda x: x != DELIMITER, data)
    data = data.split('&')
    data = prefix + data + postfix + ['EOLEOL']
    data = string.join(data, DELIMITER)
    print "%s" % (data)
    sys.stdout.flush()

  def do_POST(self):
    print >> sys.stderr, ''
    print >> sys.stderr, self.headers

    if not 'content-length' in self.headers.keys():
      print >> sys.stderr, 'No content-length in POST request'
      return
    length = int(self.headers['content-length'])

    if not 'user-agent' in self.headers.keys():
      useragent = ''
    else:
      useragent = self.headers['user-agent']

     if not 'cookie' in self.headers.keys():
       cookie = ''
       # TODO bail out if no cookie?
     else:
       cookie = self.headers['cookie']

    prefix = [str(self.client_address[0]), str(self.client_address[1]), useragent]
    data = self.rfile.read(length)
    self.parsePostdata(data, prefix)

    # return a response to the client
    text = 'Got it! Thanks!'
    self.send_response(200)
    self.send_header('Content-length', len(text))
    self.end_headers()
    self.wfile.write(text)

if __name__ == '__main__':
  print >> sys.stderr, 'starting.......'
  server_class = BaseHTTPServer.HTTPServer
  httpd = server_class(('127.0.0.1', 8888), MyHandler)
  if USESSL:
    httpd.socket = ssl.wrap_socket(httpd.socket, keyfile=KEYFILE, certfile=CERT, server_side=True)
  httpd.serve_forever()
