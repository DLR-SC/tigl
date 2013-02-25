#
# Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
#
# Created: 2012-12-17 Martin Siggel <Martin.Siggel@dlr.de>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# @file componentSegmentExample.py
# @brief Demonstrates the use of the segment and component segment geometry classes
#
import pkg_resources
pkg_resources.require("matplotlib")
from ms_componentSegmentGeom import *
from ms_segmentGeometry import *

import matplotlib as mpl
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt

# define outer point of wing segment
p1 = array([0, 0.0, -0.3])
p2 = array([2.0,5.4, 0.0])
p3 = array([3.5,0.0, 0.0])
p4 = array([3.5,6.4, 0.0])
p5 = array([3.4, 9.0, 0.4])
p6 = array([4.0, 9.5, 0.5])

CS  = ComponentSegmentGeometry(p1, p2, p3, p4)

compSeg = ComponentSegment()
compSeg.addSegment(p1, p2, p3, p4)
compSeg.addSegment(p2, p5, p4, p6)

fig = plt.figure()
ax = fig.gca(projection='3d')

compSeg.draw(ax)
compSeg.drawPoint(ax, 0.1, 0.1)
compSeg.drawPoint(ax, 0.9, 0.9)
compSeg.drawPoint(ax, 0.6, 0.4)
compSeg.drawPoint(ax, 0.6, 0.2)

seg = SegmentGeometry(p1,p2,p3,p4)

#plot iso eta and xsi lines in segment coordinates,
for alpha in arange(0,1.01,0.1):
	pp1 = seg.calcPoint(alpha,0);
	pp2 = seg.calcPoint(alpha,1);
	ax.plot([pp1[0,0], pp2[0,0]], [pp1[1,0], pp2[1,0]], [pp1[2,0], pp2[2,0]],'r');

for beta in arange(0,1.01,0.1):
	pp1 = seg.calcPoint(0,beta);
	pp2 = seg.calcPoint(1,beta);
	#ax.plot([pp1[0,0], pp2[0,0]], [pp1[1,0], pp2[1,0]], [pp1[2,0], pp2[2,0]],'r');

# plot normals
for beta in arange(0,1.01,0.1):
	normal = CS.calcCSPointNormal( 1.0, beta)
	pp1    = CS.calcCSPoint(1.0, beta)
	pp2 = pp1 + normal*0.2;
	#ax.plot([pp1[0], pp2[0]], [pp1[1], pp2[1]], [pp1[2], pp2[2]],'m');
	normal = CS.calcCSPointNormal( 0.5, beta)
	pp1    = CS.calcCSPoint(0.5, beta)
	pp2 = pp1 + normal*0.2;
	#ax.plot([pp1[0], pp2[0]], [pp1[1], pp2[1]], [pp1[2], pp2[2]],'m');
	normal = CS.calcCSPointNormal( 0, beta)
	pp1    = CS.calcCSPoint(0, beta)
	pp2 = pp1 + normal*0.2;
	ax.plot([pp1[0], pp2[0]], [pp1[1], pp2[1]], [pp1[2], pp2[2]],'m');

ax.set_zlim([-1, 1])

p = CS.calcCSPoint(0.3, 0.8)
(eta,xsi) = CS.projectOnCS(p)
p1 = CS.calcCSPoint(eta, xsi)

plt.show()
