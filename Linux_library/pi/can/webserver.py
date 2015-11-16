# webserver.py

import subprocess
from cherrypy import wsgiserver


def application(environ, start_response):
    start_response('200 OK', [('Content-Type', 'text/plain')])
    proc = subprocess.Popen(['./can_parse', ''], stdout=subprocess.PIPE)

    line = proc.stdout.readline()
    while line:
        yield line
        line = proc.stdout.readline()


server = wsgiserver.CherryPyWSGIServer(('0.0.0.0', 8000), application)
server.start()
