#!/usr/bin/python
#
# Basic script to plot dataset directly from HDF5 file
#
import mechanic_toolkit as mt

settings = {
    'filename' : 'rev11-master-00.h5',
    'dataset' : 'megno',
    'output' : 'foo.png',
    'slice' : 0,
    'cblabel' : '<Y>',
    'origin-color' : 'white',
    'origin-label' : r'$\mathbf{\bullet}$'
    }

mt.PlotDataset(settings)
