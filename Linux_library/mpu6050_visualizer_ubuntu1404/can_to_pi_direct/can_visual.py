#!/usr/bin/python

from __future__ import print_function

import pygame
import urllib
from OpenGL.GL import *
from OpenGL.GLU import *
import math
from math import radians
from pygame.locals import *

import requests
import json
import os
import time
import subprocess
import sys

import sys
import re
import boto
import argparse
import time
import datetime

from argparse import RawTextHelpFormatter
from boto.kinesis.exceptions import ProvisionedThroughputExceededException
import poster

import threading
import Queue

#For pygame/OpenGL install
#apt-get install python-virtualenv
#virtualenv /opt/myenv
#source /opt/myenv/bin/activate

#sudo apt-get install mercurial python3-pip libfreetype6-dev
#sudo apt-get build-dep python-pygame
#sudo pip install hg+http://bitbucket.org/pygame/pygame

#easy_install pyopengl
#pip install requests
#pip install json

SCREEN_SIZE = (800, 600)
SCALAR = .5
SCALAR2 = 0.2

def echo_records(records):
    for record in records:
        text = record['Data']
        #print(text)
        sys.stdout.write(text)
        #print('+--> echo record:\n{0}'.format(text))

class readCAN(threading.Thread):
    def __init__(self, workQueue, queueLock):
        super(readCAN, self).__init__()
        self.workQueue = workQueue
        self.queueLock = queueLock
        self.proc = subprocess.Popen(['can_parse', 'raw', '300'], stdout=subprocess.PIPE, bufsize=1, shell=True)

    def run(self):
        #for line in iter(self.proc.stdout.readline, b''):
            #if not line.startswith('WAIT'):
                #self.workQueue.append(line)
            #self.workQueue.put(line)
        line = self.proc.stdout.readline()
        while True:
            if not line.startswith('WAIT'):
                self.workQueue.append(line)
            line = self.proc.stdout.readline()

def resize(width, height):
    glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45.0, float(width) / height, 0.001, 10.0)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()
    gluLookAt(0.0, 1.0, -5.0,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0)
    
def init():
    glEnable(GL_DEPTH_TEST)
    glClearColor(0.0, 0.0, 0.0, 0.0)
    glShadeModel(GL_SMOOTH)
    glEnable(GL_BLEND)
    glEnable(GL_POLYGON_SMOOTH)
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST)
    glEnable(GL_COLOR_MATERIAL)
    glEnable(GL_LIGHTING)
    glEnable(GL_LIGHT0)
    glLightfv(GL_LIGHT0, GL_AMBIENT, (0.3, 0.3, 0.3, 1.0));

class readValues(threading.Thread):
	def __init__(self, workQueue, queueLock):
		super(readValues, self).__init__()
		self.workQueue = workQueue
		self.queueLock= queueLock
		#self.proc = subprocess.Popen(['python', '../pi/can_kinesis/worker.py', 'badgerloop', '--region', 'us-west-2', '--worker_time', '345600', '--sleep_interval', '3', '--echo'], stdout=subprocess.PIPE, bufsize=1)

	def run(self):
		#proc = subprocess.Popen(['python', 'worker.py', 'badgerloop_testing', '--worker_time', '345600', '--echo'], stdout=subprocess.PIPE, bufsize=1)
		#line = self.proc.stdout.readline()
		#while line:
		for line in iter(self.proc.stdout.readline, b''):
			self.workQueue.put(line.split(" "))
			#line = self.proc.stdout.readline()
				#self.queueLock.release()


#def read_values():
    #link = "http://10.0.1.7:8000" # Change this address to your settings
    #f = urllib.urlopen(link)
    #myfile = f.read()
    #line = r.iter_lines()
    #p = subprocess.Popen(['python', 'worker.py', 'badgerloop_testing', '--worker_time', '345600', '--echo'], stdout=subprocess.PIPE, bufsize=1)
    #line = p.stdout.readline()
    #while line:
    	#values = line.split(" ")
    	#line = p.stdout.readline()
    #for line in r.iter_lines():
    	#if line:
    		#return line.split(" ")

def dist(a,b):
    return math.sqrt((a*a)+(b*b))

def get_y_rotation(x,y,z):
    radians = math.atan2(x, dist(y,z))
    return -math.degrees(radians)

def get_x_rotation(x,y,z):
    radians = math.atan2(y, dist(x,z))
    return math.degrees(radians)

def run():
    pygame.init()
    screen = pygame.display.set_mode(SCREEN_SIZE, HWSURFACE | OPENGL | DOUBLEBUF)
    resize(*SCREEN_SIZE)
    init()
    clock = pygame.time.Clock()
    cube = Cube((0.0, 0.0, 0.0), (.5, .5, .7))
    angle = 0

    values = ("ax", "ay", "az")
    #work_q = Queue.Queue()
    q_lock = threading.Lock()

    #read_values = readValues(work_q, q_lock)
    #read_values.setDaemon(True)
    #threads.append(read_values)
    #read_values.start()

    x_angle = 0
    y_angle = 0
    
    while True:
        then = pygame.time.get_ticks()
        for event in pygame.event.get():
            if event.type == QUIT:
                return
            if event.type == KEYUP and event.key == K_ESCAPE:
                return

        try:
        	if not work_q.empty():
        		values = work_q.get()
        		print(values)
            	x_angle = get_x_rotation(float(values[0]), float(values[1]), float(values[2]))
            	y_angle = get_y_rotation(float(values[0]), float(values[1]), float(values[2]))
        except:
            print(values)


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        glColor((1.,1.,1.))
        glLineWidth(1)
        glBegin(GL_LINES)

        for x in range(-20, 22, 2):
            glVertex3f(x/10.,-1,-1)
            glVertex3f(x/10.,-1,1)
        
        for x in range(-20, 22, 2):
            glVertex3f(x/10.,-1, 1)
            glVertex3f(x/10., 1, 1)
        
        for z in range(-10, 12, 2):
            glVertex3f(-2, -1, z/10.)
            glVertex3f( 2, -1, z/10.)

        for z in range(-10, 12, 2):
            glVertex3f(-2, -1, z/10.)
            glVertex3f(-2,  1, z/10.)

        for z in range(-10, 12, 2):
            glVertex3f( 2, -1, z/10.)
            glVertex3f( 2,  1, z/10.)

        for y in range(-10, 12, 2):
            glVertex3f(-2, y/10., 1)
            glVertex3f( 2, y/10., 1)
        
        for y in range(-10, 12, 2):
            glVertex3f(-2, y/10., 1)
            glVertex3f(-2, y/10., -1)
        
        for y in range(-10, 12, 2):
            glVertex3f(2, y/10., 1)
            glVertex3f(2, y/10., -1)
        
        glEnd()
        glPushMatrix()
        glRotate(float(x_angle), 1, 0, 0)
        glRotate(-float(y_angle), 0, 0, 1)
        cube.render()
        glPopMatrix()
        pygame.display.flip()

class Cube(object):

    def __init__(self, position, color):
        self.position = position
        self.color = color

    # Cube information
    num_faces = 6

    vertices = [ (-1.0, -0.05, 0.5),
                 (1.0, -0.05, 0.5),
                 (1.0, 0.05, 0.5),
                 (-1.0, 0.05, 0.5),
                 (-1.0, -0.05, -0.5),
                 (1.0, -0.05, -0.5),
                 (1.0, 0.05, -0.5),
                 (-1.0, 0.05, -0.5) ]

    normals = [ (0.0, 0.0, +1.0),  # front
                (0.0, 0.0, -1.0),  # back
                (+1.0, 0.0, 0.0),  # right
                (-1.0, 0.0, 0.0),  # left
                (0.0, +1.0, 0.0),  # top
                (0.0, -1.0, 0.0) ]  # bottom

    vertex_indices = [ (0, 1, 2, 3),  # front
                       (4, 5, 6, 7),  # back
                       (1, 5, 6, 2),  # right
                       (0, 4, 7, 3),  # left
                       (3, 2, 6, 7),  # top
                       (0, 1, 5, 4) ]  # bottom

    def render(self):
        then = pygame.time.get_ticks()
        glColor(self.color)

        vertices = self.vertices

        # Draw all 6 faces of the cube
        glBegin(GL_QUADS)

        for face_no in xrange(self.num_faces):
            glNormal3dv(self.normals[face_no])
            v1, v2, v3, v4 = self.vertex_indices[face_no]
            glVertex(vertices[v1])
            glVertex(vertices[v2])
            glVertex(vertices[v3])
            glVertex(vertices[v4])
        glEnd()

if __name__ == "__main__":

    threads = []
    work_q = Queue.Queue()

    read_can = readCAN(
        workQueue = work_q
        )
    threads.append(read_can)
    read_can.start()

    run()