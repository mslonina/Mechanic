# -*- coding: utf-8 -*-
#!/usr/bin/python

import h5py
import matplotlib
from matplotlib import rc
import numpy
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1 import make_axes_locatable
from pylab import *
from matplotlib.backends.backend_pdf import PdfPages

rcParams['mathtext.fontset'] = 'stixsans'
rcParams['mathtext.default'] = 'regular'

font = {'family' : 'sans-serif'}
matplotlib.rc('font', **font)

#
# The Mechanic Python toolkit
#

#
# @function
# Plots the dataset in a PM3D-like manner
# This works for 3D STORAGE_BOARD datasets
#
def Plot3(settings):
  try:
    f = h5py.File(settings['filename'] + '.h5', 'r')
  except:
    print "Missing file: " + settings['filename']
    return

  board = f['/Pools/last/board']

  if not 'output' in settings:
    output = settings['filename'] + '.png'
    output_pdf = settings['filename'] + '.pdf'
  if not 'title' in settings:
    title = board.attrs["title"][0]
  else:
    title = settings['title']
  if not 'description' in settings:
    description = board.attrs["description"][0]
  else:
    description = settings['description']
  if not 'xlabel' in settings:
    xlabel = board.attrs["xlabel"][0]
  else:
    xlabel = settings['xlabel']
  if not 'ylabel' in settings:
    ylabel = board.attrs["ylabel"][0]
  else:
    ylabel = settings['ylabel']

  xmin = board.attrs["xmin"]
  xmax = board.attrs["xmax"]
  ymin = board.attrs["ymin"]
  ymax = board.attrs["ymax"]

  if not 'xorigin' in settings:
    xorigin = board.attrs["xorigin"]
  else:
    xorigin = settings['xorigin']
  if not 'yorigin' in settings:
    yorigin = board.attrs["yorigin"]
  else:
    yorigin = settings['yorigin']

  extent = [xmin, xmax, ymin, ymax]

  result = f['/Pools/last/Tasks/' + settings['dataset']]
  data = result[:,:,settings['slice']]

  plt.close('all')
  fig = plt.figure()

  plt.ticklabel_format(style='plain', axis='both', scilimits=(0,0))

  plt.title(title + '\n' + description)
  
  ax = fig.add_subplot(111)
  im = ax.imshow(data[::-1], aspect='auto', cmap=plt.cm.gnuplot, extent=extent)
  plt.tick_params(axis='both', which='major', labelsize=10)
  xaxis = ax.set_xlabel(xlabel, fontsize = 14)
  yaxis = ax.set_ylabel(ylabel, fontsize = 14)

  divider = make_axes_locatable(ax)
  cax = divider.append_axes("right", size="2%", pad=0.05)
  
  cbar = plt.colorbar(im,cax=cax, orientation='vertical')
  cbar.set_label(settings['cblabel'])

# the origin 

  xp = (xorigin - xmin) / (xmax - xmin)
  yp = (yorigin - ymin) / (ymax - ymin)
  ax.text(xp, yp, settings['origin-label'], color=settings['origin-color'], ha='center', va='center', transform=ax.transAxes)

# png
  savefig(output)

# pdf
  pp = PdfPages(output_pdf)
  pp.savefig()
  pp.close()

  f.close()

#
# @function
# Plots the dataset in a PM3D-like manner
# This works for 3D STORAGE_BOARD datasets
#
def Plot3V(settings):
  try:
    f = h5py.File(settings['filename'] + '.h5', 'r')
  except:
    print "Missing file: " + settings['filename']
    return

  board = f['/Pools/last/board']

  if not 'output' in settings:
    output = settings['filename'] + '.png'
    output_pdf = settings['filename'] + '.pdf'
  if not 'title' in settings:
    title = board.attrs["title"][0]
  else:
    title = settings['title']
  if not 'description' in settings:
    description = board.attrs["description"][0]
  else:
    description = settings['description']
  if not 'xlabel' in settings:
    xlabel = board.attrs["xlabel"][0]
  else:
    xlabel = settings['xlabel']
  if not 'ylabel' in settings:
    ylabel = board.attrs["ylabel"][0]
  else:
    ylabel = settings['ylabel']

  xmin = board.attrs["xmin"]
  xmax = board.attrs["xmax"]
  ymin = board.attrs["ymin"]
  ymax = board.attrs["ymax"]

  if not 'xorigin' in settings:
    xorigin = board.attrs["xorigin"]
  else:
    xorigin = settings['xorigin']
  if not 'yorigin' in settings:
    yorigin = board.attrs["yorigin"]
  else:
    yorigin = settings['yorigin']

  extent = [xmin, xmax, ymin, ymax]

  result = f['/Pools/last/Tasks/' + settings['dataset']]
  data = result[:,:,settings['slice']]

  plt.close('all')
  fig = plt.figure()

  plt.ticklabel_format(style='plain', axis='both', scilimits=(0,0))

  plt.title(title + '\n' + description)
  
  ax = fig.add_subplot(111)
  im = ax.imshow(data[::-1], aspect='auto', cmap=plt.cm.gnuplot, extent=extent)
  plt.tick_params(axis='both', which='major', labelsize=10)
  xaxis = ax.set_xlabel(xlabel, fontsize = 14)
  yaxis = ax.set_ylabel(ylabel, fontsize = 14)

  divider = make_axes_locatable(ax)
  cax = divider.append_axes("right", size="2%", pad=0.05)
  
  cbar = plt.colorbar(im,cax=cax, orientation='vertical')
  cbar.set_label(settings['cblabel'])

# the origin 

  xp = (xorigin - xmin) / (xmax - xmin)
  yp = (yorigin - ymin) / (ymax - ymin)
  ax.text(xp, yp, settings['origin-label'], color=settings['origin-color'], ha='center', va='center', transform=ax.transAxes)


  plt.show()
  f.close()

#
# @function
# Plots the dataset in a PM3D-like manner
# This works for 3D STORAGE_BOARD datasets
#
# No colorbar here
#
def Plot3CB(settings):
  try:
    f = h5py.File(settings['filename'] + '.h5', 'r')
  except:
    print "Missing file: " + settings['filename']
    return

  board = f['/Pools/last/board']

  if not 'output' in settings:
    output = settings['filename'] + '.png'
    output_pdf = settings['filename'] + '.pdf'
  if not 'title' in settings:
    title = board.attrs["title"][0]
  else:
    title = settings['title']
  if not 'description' in settings:
    description = board.attrs["description"][0]
  else:
    description = settings['description']
  if not 'xlabel' in settings:
    xlabel = board.attrs["xlabel"][0]
  else:
    xlabel = settings['xlabel']
  if not 'ylabel' in settings:
    ylabel = board.attrs["ylabel"][0]
  else:
    ylabel = settings['ylabel']

  xmin = board.attrs["xmin"]
  xmax = board.attrs["xmax"]
  ymin = board.attrs["ymin"]
  ymax = board.attrs["ymax"]

  if not 'xorigin' in settings:
    xorigin = board.attrs["xorigin"]
  else:
    xorigin = settings['xorigin']
  if not 'yorigin' in settings:
    yorigin = board.attrs["yorigin"]
  else:
    yorigin = settings['yorigin']

  extent = [xmin, xmax, ymin, ymax]

  result = f['/Pools/last/Tasks/' + settings['dataset']]
  data = result[:,:,settings['slice']]

  plt.close('all')
  fig = plt.figure()

  plt.ticklabel_format(style='sci', axis='both', scilimits=(0,0))

#  plt.title(title + '\n' + description)
  
  ax = fig.add_subplot(111)
  im = ax.imshow(data[::-1], aspect='auto', cmap=plt.cm.gnuplot, extent=extent)
  plt.tick_params(axis='both', which='major', labelsize=14)
#  xaxis = ax.set_xlabel(xlabel, fontsize = 14)
#  yaxis = ax.set_ylabel(ylabel, fontsize = 14)

#  divider = make_axes_locatable(ax)
#  cax = divider.append_axes("right", size="2%", pad=0.05)
  
#  cbar = plt.colorbar(im,cax=cax, orientation='vertical')
#  cbar.set_label(settings['cblabel'])

# the origin 

  xp = (xorigin - xmin) / (xmax - xmin)
  yp = (yorigin - ymin) / (ymax - ymin)
  #settings['origin-label'] = "A"
  ax.text(xp, yp, settings['origin-label'], color=settings['origin-color'], fontsize=20, ha='center', va='center', transform=ax.transAxes)

# png
  savefig(output)

# pdf
  pp = PdfPages(output_pdf)
  pp.savefig()
  pp.close()

  f.close()


