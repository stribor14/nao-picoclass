#!/usr/bin/python
import os
import random
import cv2
import struct
import sys
import numpy as np
import PythonMagick as pm
import progressbar

def choosePicture(dirName) : # loads random picture from directory tree
	files = [os.path.join(path, filename)
         for path, dirs, files in os.walk(dirName)
         for filename in files
         if not filename.endswith(".bak")]
	temp = cv2.imread(random.choice(files))
	if temp is None :
		return choosePicture(dirName)
	else :
		return temp[:,:,0]

def merge(picF, picB) : # scales 'picF' so it is smaller than 'picB' and pastes it on random position
	sizeF = picF.shape
	sizeB = picB.shape
	

	scale = np.uint8(random.uniform(min(sizeB)/5, min(sizeB)/2))
	while scale == 0 : # sometimes it gets size 0 ... and this helps... dunno why o_O
		scale = np.uint8(random.uniform(min(sizeB)/5, min(sizeB)/2))

	if scale > min(sizeF) :
		picF = cv2.resize(picF,(scale,scale), interpolation = cv2.INTER_CUBIC)
		sizeF = picF.shape
	else :
		picF = cv2.resize(picF,(scale,scale), interpolation = cv2.INTER_AREA)
		sizeF = picF.shape

	x = random.choice(range(sizeB[0] - scale))
	y = random.choice(range(sizeB[1] - scale))
	

	for k in range(scale) :
		for i in range(scale) :
			if picF[k][i] > 0 :
				picB[k+x][i+y] = picF[k][i]
			
	return picB

def  main() :
	origPic = choosePicture("/home/mirko/Desktop/sve_/zaba/")
	backPic = choosePicture("/home/mirko/Devel/ADORE/PicoClass/Pictures/backgroundimages")
	mergedPic = merge(origPic, backPic)
	cv2.imwrite("/home/mirko/Desktop/sve_/test.png",mergedPic)

if __name__ == "__main__" :
	main()