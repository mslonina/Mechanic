# -*- coding: utf-8 -*-
#!/usr/bin/python

import h5py
import matplotlib
import numpy
import matplotlib.pyplot as plt
from pylab import *

#rc('text', usetex=True)

#
# The Mechanic Python toolkit
#

#
# @function
# Plots the dataset in a PM3D-like manner
#
def PlotDataset(settings):
  f = h5py.File(settings['filename'], 'r')

  board = f['/Pools/last/board']
  title = board.attrs["title"][0]
  description = board.attrs["description"][0]
  xlabel = board.attrs["xlabel"][0]
  ylabel = board.attrs["ylabel"][0]
  xmin = board.attrs["xmin"]
  xmax = board.attrs["xmax"]
  ymin = board.attrs["ymin"]
  ymax = board.attrs["ymax"]
  xorigin = board.attrs["xorigin"]
  yorigin = board.attrs["yorigin"]

  extent = [xmin, xmax, ymin, ymax]
  print extent

  result = f['/Pools/last/Tasks/' + settings['dataset']]
  data = result[:,:,settings['slice']]

  fig = plt.figure()

  plt.ticklabel_format(style='plain', axis='both', scilimits=(0,0))

  plt.title(title + '\n' + description)
  plt.xlabel(xlabel)
  plt.ylabel(ylabel)
  im = plt.imshow(data[::-1], cmap=plt.cm.gnuplot, extent=extent)
  cb = plt.colorbar(im)
  cb.set_label(settings['cblabel'])

# the origin 
  bx = fig.add_subplot(111)
  xp = (xorigin - xmin) / (xmax - xmin)
  yp = (yorigin - ymin) / (ymax - ymin)
  print xp, yp
  text(xp, yp, settings['origin-label'], color=settings['origin-color'], ha='center', va='center', transform=bx.transAxes)

  savefig(settings['output'])

  f.close()


