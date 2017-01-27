#!/usr/bin/python

import argparse
import cv2
from scipy.spatial import distance
import os
 
refPt = []
refD = 0
image = []
cropping = False

# draw selected picture and its selection
def draw_pic(pt1, d):
    global image
    image = clone.copy() 
    cv2.rectangle(image, (pt1[0] - d, pt1[1] - d), (pt1[0] + d, pt1[1] + d), (0, 255, 0), 2)
    cv2.imshow("image", image)
    
    selection = clone[pt1[1]-d:pt1[1]+d, pt1[0]-d:pt1[0]+d]
   # cv2.imshow("selection", selection)
     
        
# open image list
ap = argparse.ArgumentParser()
ap.add_argument('src', help="Path to the image folder containing im_list.txt")
args = ap.parse_args()
imList = open(args.src + '/nazivi.txt', 'r').readlines()
rs = [float(line.split()[1]) for line in open(args.src+'/anot.txt', 'r').readlines()]
cs = [float(line.split()[0]) for line in open(args.src+'/anot.txt', 'r').readlines()]
ss = [float(line.split()[2]) for line in open(args.src+'/anot.txt', 'r').readlines()]

# iterate through list
for k in range(0, len(imList)):
    image = cv2.imread(args.src + "/pozitivneRGB/" + imList[k].strip())
    clone = image.copy()
    cv2.namedWindow("image")
    cv2.moveWindow("image", 500, 500)
    cv2.imshow("image",image)
    refPt = [int(cs[k]), int(rs[k])]
    refD = int(ss[k])
    draw_pic(refPt, refD)
    

    while True:
        cv2.imshow("image", image)
        key = cv2.waitKey(1) & 0xFF
        
        elif key == ord("c"):
            cv2.destroyAllWindows()
            break
