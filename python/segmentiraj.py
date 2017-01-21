#!/usr/bin/python
import os
import random
import cv2
import struct
import sys
import numpy as np
import PythonMagick as pm
import progressbar

def doThatOnPictures(inDirName, outDirName) : # loads random picture from directory tree
	files = [os.path.join(path, filename)
         for path, dirs, files in os.walk(inDirName)
         for filename in files
         if not filename.endswith(".bak")]
	for filename, index in zip(files, range(len(files))) :
		tempColor = cv2.imread(filename)
		tempGS = cv2.imread(filename, 0)
		#tempGS = cv2.GaussianBlur(tempGS,(3,3),0)
		ret,tempBW = cv2.threshold(tempGS,0,255,cv2.THRESH_BINARY_INV+cv2.THRESH_OTSU)
		tempOut = np.zeros(tempColor.shape, np.uint8)
		if tempColor is not None and tempBW is not None :
			for k in range(tempBW.shape[0]) :
				for i in range(tempBW.shape[1]) :
					if tempBW[k][i] :
						tempOut[k,i,:] = tempColor[k,i,:]
		cv2.imwrite(outDirName + "cut_" + str(index) + ".png", tempOut)

def  main() :
	origPic = doThatOnPictures("/home/mirko/Desktop/sve_/zaba/", "/home/mirko/Desktop/sve_/zaba_cut/")


if __name__ == "__main__" :
	main()