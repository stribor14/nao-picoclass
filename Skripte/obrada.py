
#
#

#
import sys
import random
import numpy
from scipy import misc
from PIL import Image
from PIL import ImageOps
import struct
import argparse
import os

itera = 0

#
parser = argparse.ArgumentParser()
parser.add_argument('src', help='source folder')
args = parser.parse_args()

#
src = args.src

#
plot = 1

if plot:
	import matplotlib.pyplot
	import matplotlib.image
	import matplotlib.cm

#
def write_rid(im):
	#
	# raw intensity data
	#

	#
	h = im.shape[0]
	w = im.shape[1]

	#
	hw = struct.pack('ii', h, w)

	tmp = [None]*w*h
	for y in range(0, h):
		for x in range(0, w):
			tmp[y*w + x] = im[y, x]

	#
	pixels = struct.pack('%sB' % w*h, *tmp)

	#
	sys.stdout.buffer.write(hw)
	sys.stdout.buffer.write(pixels)

#
def export(im, r, c, s):
	#
	nrows = im.shape[0]
	ncols = im.shape[1]

	# crop
	r0 = max(int(r - s), 0); r1 = min(r + s, nrows)
	c0 = max(int(c - s), 0); c1 = min(c + s, ncols)

	im = im[r0:r1, c0:c1]

	nrows = im.shape[0]
	ncols = im.shape[1]

	r = r - r0
	c = c - c0

	# resize, if needed
	maxwsize = 192.0
	wsize = max(nrows, ncols)

	ratio = maxwsize/wsize

	if ratio<1.0:
		im = numpy.asarray( Image.fromarray(im).resize((int(ratio*ncols), int(ratio*nrows))) )

		r = ratio*r
		c = ratio*c
		s = ratio*s

	#
	nrands = 7;

	lst = []

	for i in range(0, nrands):
		#
		stmp = s*random.uniform(0.9, 1.1)

		rtmp = r + s*random.uniform(-0.05, 0.05)
		ctmp = c + s*random.uniform(-0.05, 0.05)

		#
		if plot:
			matplotlib.pyplot.cla()

			matplotlib.pyplot.plot([ctmp-stmp, ctmp+stmp], [rtmp-stmp, rtmp-stmp], 'b', linewidth=3)
			matplotlib.pyplot.plot([ctmp+stmp, ctmp+stmp], [rtmp-stmp, rtmp+stmp], 'b', linewidth=3)
			matplotlib.pyplot.plot([ctmp+stmp, ctmp-stmp], [rtmp+stmp, rtmp+stmp], 'b', linewidth=3)
			matplotlib.pyplot.plot([ctmp-stmp, ctmp-stmp], [rtmp+stmp, rtmp-stmp], 'b', linewidth=3)

			matplotlib.pyplot.imshow(im, cmap=matplotlib.cm.Greys_r)

			matplotlib.pyplot.show()

		lst.append( (int(rtmp), int(ctmp), int(stmp)) )

	#
	write_rid(im)

	sys.stdout.buffer.write( struct.pack('i', nrands) )

	for i in range(0, nrands):
		sys.stdout.buffer.write( struct.pack('iii', lst[i][0], lst[i][1], lst[i][2]) )

def mirror_and_export(im, r, c, s):
	#
	# exploit mirror symmetry of the face
	#

	# flip image along vertical axis
	im = numpy.asarray(ImageOps.mirror(Image.fromarray(im)))
	# flip column coordinate of the object
	c = im.shape[1] - c
	# export
	export(im, r, c, s)
	

# image list

imlist = open(src + '/im_list.txt', 'r').readlines()

# object sample is specified by three coordinates (row, column and size; all in pixels)
rs = [float(line.split()[1]) for line in open(src+'/im_labels2.txt', 'r').readlines()]
cs = [float(line.split()[0]) for line in open(src+'/im_labels2.txt', 'r').readlines()]
ss = [float(line.split()[2]) for line in open(src+'/im_labels2.txt', 'r').readlines()]

#
n = 0

for i in range(0, len(rs)):
	# construct full image path
	path = src + '/zaba/' + imlist[i].strip()

	r = rs[i]
	c = cs[i]
	s = ss[i]

	#
	try:
		im = Image.open(path).convert('L')
	except:
		continue

	#
	im = numpy.asarray(im)

	#
	export(im, r, c, s)

	# faces are symmetric and we exploit this here
	mirror_and_export(im, r, c, s)

