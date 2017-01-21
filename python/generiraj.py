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


def transform(pic) :
	# TODO : add "finger" covering the frog
	# TODO : change gradient with PythonMagick

	# AFFINE TRANSFORM
	h = pic.shape[0]
	w = pic.shape[1]
	picOut = np.zeros((h*2,w*2), np.uint8)
	picOut[h/2:h+h/2, w/2:w+w/2] = pic

	do_affine = 1
	if do_affine == 1 :
		rnd = lambda : random.uniform(-100,100)/800.0
		srcPt = np.float32([[0,0],[1,0],[0,1]])
		dstPt = np.float32([[rnd(),rnd()],[1+rnd(),rnd()],[rnd(),rnd()+1]])
		warpMat = cv2.getAffineTransform(srcPt,dstPt)
		picOut = cv2.warpAffine(picOut,warpMat,picOut.shape)

	do_rotation = 1
	if do_rotation == 1 :
		angle = random.uniform(1,360)
		rotMat = cv2.getRotationMatrix2D((h,w), angle, 1)
		picOut = cv2.warpAffine(picOut, rotMat, picOut.shape)

	bbox = cv2.boundingRect(picOut)
	if bbox[2] > bbox[3] :
		temp = (bbox[2]-bbox[3])/2
		picOut = picOut[bbox[1]-temp:bbox[1]+bbox[2], bbox[0]:bbox[0]+bbox[2]]
	else :
		temp = (bbox[3]-bbox[2])/2
		picOut = picOut[bbox[1]:bbox[1]+bbox[3], bbox[0]-temp:bbox[0]+bbox[3]]

	if picOut.shape[0] * picOut.shape[1] == 0 :
		return transform(pic)
	else :
		return picOut


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

	x = random.choice(range(sizeB[0] - scale -1))
	y = random.choice(range(sizeB[1] - scale -1))
	
	picM = np.zeros((scale,scale), np.uint8)

	for k in range(scale-1) :
		for i in range(scale-1) :
			if picF[k][i] > 0 :
				picM[k][i] = picF[k][i]
			else :
				picM[k][i] = picB[k+x][i+y]

	return picM

def write_rid(im):
	h = im.shape[0]
	w = im.shape[1]

	hw = struct.pack('ii', h, w)

	tmp = [None]*w*h
	for y in range(h):
		for x in range(w):
			tmp[y*w + x] = im[y, x]

	pixels = struct.pack('%sB' % w*h, *tmp)

	sys.stdout.write(hw)
	sys.stdout.write(pixels)

def export(im, s) :
	h = im.shape[0]
	w = im.shape[1]

	maxwsize = 192.0
	wsize = max(h, w)

	nrands = 7;
	lst = []

	for i in range(nrands):
		#
		stmp = s*random.uniform(0.9, 1.1)

		rtmp = h + s*random.uniform(-0.05, 0.05)
		ctmp = w + s*random.uniform(-0.05, 0.05)

		lst.append( (int(rtmp), int(ctmp), int(stmp)) )

	write_rid(im)

	sys.stdout.write( struct.pack('i', nrands) )

	for i in range(nrands):
		sys.stdout.write( struct.pack('iii', lst[i][0], lst[i][1], lst[i][2]) )

def  main() :
	bar = progressbar.ProgressBar()
	for k in bar(range(1000)):
		origPic = choosePicture("/home/mirko/Desktop/sve_/zaba/")
		backPic = choosePicture("/home/mirko/Devel/ADORE/PicoClass/Pictures/backgroundimages")
		frontPic = transform(origPic)
		mergedPic = merge(frontPic, backPic)
		export(mergedPic[0], mergedPic[1])
		#cv2.imwrite("/home/mirko/test/t" + str(k) + ".png",mergedPic[0])

if __name__ == "__main__" :
	main()