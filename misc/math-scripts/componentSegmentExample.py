from numpy import array, dot
from ms_componentSegmentGeom import *
from ms_segmentGeometry import *

import matplotlib as mpl
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt

# define outer point of wing segment
p1 = array([0,0,0])
p2 = array([1,2,0.])
p3 = array([3.5,0.3500,0])
p4 = array([3.5,3.5,0.5])

CS = ComponentSegmentGeometry(p1, p2, p3, p4, 0., 1.0)

fig = plt.figure()
ax = fig.gca(projection='3d')

#plot iso eta and xsi lines in segment coordinates, and wing outline
for alpha in arange(0,1.01,0.1):
	pp1 = ms_calcSegmentPoint(p1,p2,p3,p4,alpha,0);
	pp2 = ms_calcSegmentPoint(p1,p2,p3,p4,alpha,1);
	ax.plot([pp1[0,0], pp2[0,0]], [pp1[1,0], pp2[1,0]], [pp1[2,0], pp2[2,0]],'r');
	X,Y,Z = CS.calcCSIsoEtaLine(alpha)
	ax.plot(X,Y,Z,'g');

for beta in arange(0,1.01,0.1):
	pp1 = ms_calcSegmentPoint(p1,p2,p3,p4,0,beta);
	pp2 = ms_calcSegmentPoint(p1,p2,p3,p4,1,beta);
	X,Y,Z = CS.calcCSIsoXsiLine(beta)
	ax.plot(X,Y,Z,'g');
	ax.plot([pp1[0,0], pp2[0,0]], [pp1[1,0], pp2[1,0]], [pp1[2,0], pp2[2,0]],'r');

lw = 2;
ax.plot([p1[0], p2[0]], [p1[1], p2[1]], [p1[2], p2[2]],'b',linewidth=lw);
ax.plot([p1[0], p3[0]], [p1[1], p3[1]], [p1[2], p3[2]],'b',linewidth=lw);
ax.plot([p4[0], p2[0]], [p4[1], p2[1]], [p4[2], p2[2]],'b',linewidth=lw);
ax.plot([p3[0], p4[0]], [p3[1], p4[1]], [p3[2], p4[2]],'b',linewidth=lw);

# plot normals
for beta in arange(0,1.01,0.1):
	normal = CS.calcCSPointNorm( 1.0, beta)
	pp1    = CS.calcCSPoint(1.0, beta)
	pp2 = pp1 + normal*0.2;
	ax.plot([pp1[0], pp2[0]], [pp1[1], pp2[1]], [pp1[2], pp2[2]],'m');
	normal = CS.calcCSPointNorm( 0.5, beta)
	pp1    = CS.calcCSPoint(0.5, beta)
	pp2 = pp1 + normal*0.2;
	ax.plot([pp1[0], pp2[0]], [pp1[1], pp2[1]], [pp1[2], pp2[2]],'m');
	normal = CS.calcCSPointNorm( 0, beta)
	pp1    = CS.calcCSPoint(0, beta)
	pp2 = pp1 + normal*0.2;
	ax.plot([pp1[0], pp2[0]], [pp1[1], pp2[1]], [pp1[2], pp2[2]],'m');

ax.set_zlim([-1, 1])

p = CS.calcCSPoint(0.3, 0.8)
(eta,xsi) = CS.projectOnCS(p)
p1 = CS.calcCSPoint(eta, xsi)


plt.show()
