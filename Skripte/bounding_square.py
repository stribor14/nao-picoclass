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
    cv2.imshow("selection", selection)
    
# mouse selection
def draw_rect(event, x, y, flags, param):
    global refPt, refD, cropping

    if event == cv2.EVENT_LBUTTONDOWN:
        refPt = list((x, y))
        cropping = True  

    elif event == cv2.EVENT_LBUTTONUP:
        refD = int(distance.euclidean(refPt, (x,y)))
        cropping = False
        draw_pic(refPt, refD)
                
    elif cropping:
        draw_pic(refPt, int(distance.euclidean(refPt, (x,y))))
        
# move & resize square
def move_pic(key):
    global refPt, refD
    
    if key == 1:
        refPt[0] -= 1
        refPt[1] += 1
    elif key == 2:
        refPt[1] += 1
    elif key == 3:
        refPt[0] += 1
        refPt[1] += 1
    elif key == 4:
        refPt[0] -= 1
    elif key == 6:
        refPt[0] += 1
    elif key == 7:
        refPt[0] -= 1
        refPt[1] -= 1
    elif key == 8:
        refPt[1] -= 1
    elif key == 9:
        refPt[0] += 1
        refPt[1] -= 1
    elif key+48 == ord("+"):
        refD += 1
    elif key+48 == ord("-"):
        refD -= 1
        
    draw_pic(refPt, refD)
  
        
# open image list
ap = argparse.ArgumentParser()
ap.add_argument('src', help="Path to the image folder containing im_list.txt")
args = ap.parse_args()
imList = open(args.src + "/im_list.txt", 'r').readlines()
if os.path.isfile(args.src + "/im_labels2.txt"):
    os.remove(args.src + "/im_labels2.txt")

print "Controls:"
print "\tmouse:"
print "\t\tclick\tcenter of square"
print "\t\trelease\tsize of square"
print "\tkeyboard:"
print "\t\tnumpad\tmove square"
print "\t\t'+'/'-'\tresize square"
print "\t\t'r'\treset bounding square"
print "\t\t'c'\twrite data and continue to next picture"

# iterate through list
for k in range(1, len(imList)):
    image = cv2.imread(args.src + "/" + imList[k].strip())
    cv2.namedWindow("image")
    cv2.moveWindow("image", 500, 500)
    cv2.setMouseCallback("image", draw_rect)
    cv2.imshow("image", image)
    clone = image.copy()

    while True:
        cv2.imshow("image", image)
        key = cv2.waitKey(1) & 0xFF
        
        if ((key < 58 and key >48) or key == ord("+") or key == ord("-")) and len(refPt)==2:
            move_pic(key-48)
        
        elif key == ord("r"):
            image = clone.copy()
     
        elif key == ord("c"):
            if len(refPt) == 2:
                with open(args.src + "/im_labels2.txt", "a") as myfile:
                    myfile.write("%s %s %s\n" % (refPt[0], refPt[1], refD))
                cv2.destroyAllWindows()
                break
            else:
                print "No bounding rectangle selected!"
