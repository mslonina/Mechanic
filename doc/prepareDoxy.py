#!/usr/bin/env python
#Prepare documentation file for Doxygen
#Very poor written, I know, but I needed fast solution

import os, string

# Process additional Doxy Tag
def processDoxyTag(l):

  line = l.strip() 
  doxytag = line.lstrip("/* [")
  doxytag = doxytag.rstrip("] */")
  return doxytag

# Process single line
def processDoxyLine(line):
  
  include = 0

  l = line.strip()
  l = l.lstrip(" *")
  spaces = line.strip()
  spaces = spaces.split("@")
  si = ""
  for i in range(len(spaces[0])-1):
    si = si + " "

  tag = l.split()
  if len(tag) > 0:
    if tag[0] == "@idoc":
      type = 0
    if tag[0] == "@icode":
      type = 1

    if tag[0] == "@idoc" or tag[0] == "@icode":
      exfile = "../src/" + tag[1]
      extag = tag[2]
      includeDoxyBlock(exfile, extag, type, si)
      include = 1
    else:
      include = 0

  if not line.isspace():
     line = line.lstrip()
     line = line.lstrip("*")
     if include == 0:
       print line.rstrip()

# If defined, include some external documentation blocks 
def includeDoxyBlock(exfile, tag, type, spacesBefore):
  beginBlock = 0
  endBlock = 0
  tagMatch = 0
  closeTag = 0

  fe = open(exfile,"ro")
  
  for l in fe:
    beginBlock = 0
    endBlock = 0

    line = l.strip()

    if line[:5] == "/* [/":
      endBlock = 1
      tagMatch = 0
      closeTag = 1
    else:
      endBlock = 0
      closeTag = 0
    
    if line[:4] == "/* [" and closeTag == 0:
      beginBlock = 1
      doxytag = processDoxyTag(line)
      if tag == doxytag:
        tagMatch = 1
      else:
        tagMatch = 0
    else:
      beginBlock = 0
    

    if tagMatch == 1:
      if not beginBlock:
        if type == 0:
          if checkIfDoc(line) == -1:
            processDoxyLine(l)
          else:
            print ""
        if type == 1:
          if l.isspace():
            print ""
          else:
            if not (line[:2] == "/*" or line[:1] == "*"): 
              print spacesBefore + l.rstrip()


  fe.close()

  return

def checkIfDoc(line):
  if line[:3] == "/**":
    type = 0 # begin of doxy block
  elif line[:2] == "*/":
    type = 1 # end of doxy block
  else:
    type = -1
  return type

# Process single file
def processDoxyFile(file):
  
  doxyblock = 0
  doxybegin = 0
  doxyend = 0
  
  f = open(file, "ro")

  print """/* AUTOGENERATED FOR DOXYGEN -- DO NOT EDIT */\n"""
  print "/*!"
  for l in f:
  
    doxybegin = 0
    doxyend = 0
    line = l.strip()
  
    if checkIfDoc(line) == 0:
      doxyblock = 1
      doxybegin = 1
  
    if checkIfDoc(line) == 1:
      doxyblock = 0
      doxyend = 1
  
    if doxyblock == 1:
      if doxybegin == 1 or doxyend == 1:
        print ""
      else:
        processDoxyLine(l)

  print "*/"
  f.close()


processDoxyFile("../src/mechanic.c")
