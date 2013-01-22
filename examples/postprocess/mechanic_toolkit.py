# -*- coding: utf-8 -*-
#!/usr/bin/python

import h5py
import matplotlib
import numpy
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1 import make_axes_locatable
from pylab import *

#
# The Mechanic Python toolkit
#

#
# @function
# Plots the dataset in a PM3D-like manner
# This works for 3D STORAGE_BOARD datasets
#
def Plot3(settings):
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

  result = f['/Pools/last/Tasks/' + settings['dataset']]
  data = result[:,:,settings['slice']]

  plt.close('all')
  fig = plt.figure()

  plt.ticklabel_format(style='plain', axis='both', scilimits=(0,0))

  plt.title(title + '\n' + description)
  
  ax = fig.add_subplot(111)
  im = ax.imshow(data[::-1], aspect='auto', cmap=plt.cm.gnuplot, extent=extent)
  xaxis = ax.set_xlabel(xlabel, fontsize = 7)
  yaxis = ax.set_ylabel(ylabel, fontsize = 7)

  divider = make_axes_locatable(ax)
  cax = divider.append_axes("right", size="2%", pad=0.05)
  
  cbar = plt.colorbar(im,cax=cax, orientation='vertical')
  cbar.set_label(settings['cblabel'])

# the origin 
  xp = (xorigin - xmin) / (xmax - xmin)
  yp = (yorigin - ymin) / (ymax - ymin)
  ax.text(xp, yp, settings['origin-label'], color=settings['origin-color'], ha='center', va='center', transform=ax.transAxes)

  savefig(settings['output'])

  f.close()


