#!/usr/bin/env python

import BaseHTTPServer
from Cookie import SimpleCookie
import random
import string

COOKIE_LENGTH = 52
SESSION_ID = 'sess-id'

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
  def send_survey(self):
    print ''
    print self.headers

    self.send_response(200)

    # handle cookies
    cookie = None
    if 'cookie' in self.headers.keys():
      cookies = SimpleCookie(self.headers['cookie'])
      cookie = cookies[SESSION_ID].value
    if cookie == None:
      cookie = ''.join(random.choice(string.ascii_uppercase + string.ascii_lowercase + string.digits) for x in range(COOKIE_LENGTH))
      self.send_header('Set-Cookie', '%s=%s' % (SESSION_ID, cookie))

    self.end_headers()
    self.wfile.write("asdf")

  def do_GET(self):
    if self.path in self.urls.keys():
      self.urls[self.path](self)
    else:
      self.send_response(404)


  urls = {
    '/': send_survey,
  }

if __name__ == '__main__':
  print 'starting.......'
  server_class = BaseHTTPServer.HTTPServer
  httpd = server_class(('127.0.0.1', 8888), MyHandler)
  httpd.serve_forever()
