#
# Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
#
# Created: 2013-04-19 Martin Siggel <Martin.Siggel@dlr.de>
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
# @file ComponentSegmentNew.py
# @brief Implementation of the component segment geometry
#

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

from Polygonzug import PolygonNormal, PolygonWRoundedEdges
from ms_segmentGeometry import SegmentGeometry


from numpy import size,  array, dot

class ComponentSegment:
    def __init__(self, le, te):
        assert(size(le,1) == size(te,1))
        assert(size(le,0) == size(te,0) == 3)
        assert(size(le,1) >= 2)
        
        self.lepoints = le.copy()
        self.tepoints = te.copy()
        
        self.segments = {}
        nseg = size(le,1)-1
        for iseg in range(0,nseg):
            self.segments[iseg] = SegmentGeometry(self.lepoints[:,iseg], self.lepoints[:,iseg+1], self.tepoints[:,iseg], self.tepoints[:,iseg+1])
        
        # extend leading edge at wing tip 
        n = le[:,nseg]-le[:,nseg-1]
        n[0] = 0
        tep = te[:,nseg]
        alpha = dot(tep-le[:,nseg-1],n)/dot(le[:,nseg]-le[:,nseg-1],n)
        if alpha > 1:
            P = le[:,nseg-1] + (le[:,nseg]-le[:,nseg-1])*alpha
            le[:,nseg] = P
        
        # extend leading edge at inner wing
        n = le[:,1]-le[:,0]
        n[0] = 0
        tep = te[:,0]
        alpha = dot(tep-le[:,0],n)/dot(le[:,1]-le[:,0],n)
        if alpha < 0:
            P = le[:,0] + (le[:,1]-le[:,0])*alpha
            le[:,0] = P      
        
        # project onto y-z plane
        self.le = PolygonWRoundedEdges(le[1:3,:])
        self.le.setRadius(0.01)
        self.te = PolygonNormal(te[1:3,:]) 
        
        
    def calcPoint(self, eta, xsi):
        nseg = size(self.lepoints,1)-1
        pyz, nyz, iSegBegin = self.le.calcPoint(eta)
        P = array([0, pyz[0], pyz[1]])
        N = array([0, nyz[0], nyz[1]])
        
        # calculate intersection with leading edge of segment
        PV = self.segments[iSegBegin].calcIntersectLE(P,N);
        
        _ , _ , iSegEnd = self.te.calcPoint(eta)
        PH = self.segments[iSegEnd].calcIntersectTE(P,N);
        
        # calculate point on line between leading and trailing edge
        PL = PV + (PH-PV)*xsi        
        
        if iSegEnd < iSegBegin:
            iSegBegin, iSegEnd = iSegEnd, iSegBegin
            
        # project point onto segment
        for iseg in  range(iSegBegin, iSegEnd+1):
            (alpha, beta) = self.segments[iseg].projectPointOnCut(PL, P,N)
            
            # in the last and first segment, alpha and beta dont have to be valid, due to the extension of the leading edge
            if SegmentGeometry.isValid(alpha, beta):
                return self.segments[iseg].getPoint(alpha, beta)[:,0]
            elif iseg == 0 and alpha < 0.:
                return self.segments[iseg].getPoint(alpha, beta)[:,0]
            elif iseg == nseg-1 and alpha > 1.:
                 return self.segments[iseg].getPoint(alpha, beta)[:,0]
        
        raise NameError('Error determining segment index in ComponentSegment.calcPoint')
        
    def plot(self, axis):
        nseg = size(self.lepoints, 1) - 1
        style= 'b-'
        axis.plot(self.lepoints[0,:], self.lepoints[1,:], self.lepoints[2,:], style)
        axis.plot(self.tepoints[0,:], self.tepoints[1,:], self.tepoints[2,:], style)
        axis.plot([self.lepoints[0,0], self.tepoints[0,0]], [self.lepoints[1,0], self.tepoints[1,0]], [self.lepoints[2,0], self.tepoints[2,0]], style)
        axis.plot([self.lepoints[0,nseg], self.tepoints[0,nseg]], [self.lepoints[1,nseg], self.tepoints[1,nseg]], [self.lepoints[2,nseg], self.tepoints[2,nseg]], style)
        


vk = array([[0.0,  2.0, 3.4],
            [0.5,  5.4, 9.0], 
            [0.0,  0.0, 0.4]])

hk = array([[3.5, 3.5, 4.0],
            [0.0, 6.4, 9.5], 
            [0.0, 0.0, 0.5]])

cs = ComponentSegment(vk, hk)
P = cs.calcPoint(0.0, 1.0)
print P


fig = plt.figure()
fig.gca(projection='3d')

cs.plot(fig.gca())
fig.gca().plot([P[0]], [P[1]], [P[2]], 'rx')


plt.show()