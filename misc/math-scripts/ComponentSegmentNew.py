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

import Polygonzug as PZ
from ms_segmentGeometry import SegmentGeometry, SegmentMathError


from numpy import size,  array, dot, arange, zeros, linspace, linalg

class ComponentSegment(object):
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
        self.le = PZ.PolygonWRoundedEdges(le[1:3,:])
        self.le.setRadius(0.01)
        self.te = PZ.PolygonNormal(te[1:3,:]) 
        
        
    def calcPoint(self, eta, xsi):
        nseg = size(self.lepoints,1)-1
        pyz, nyz, iSegBegin = self.le.calcPoint(eta)
        #print 'proj:', self.le.project(array([pyz[0], pyz[1]])), eta
        
        P = array([0, pyz[0], pyz[1]])
        N = array([0, nyz[0], nyz[1]])
        
        # calculate intersection with leading edge of segment
        PV = self.segments[iSegBegin].calcIntersectLE(P,N)
        
        _ , _ , iSegEnd = self.te.calcPoint(eta)
        PH = self.segments[iSegEnd].calcIntersectTE(P,N)
        
        # calculate point on line between leading and trailing edge
        PL = PV + (PH-PV)*xsi        
        
        if iSegEnd < iSegBegin:
            iSegBegin, iSegEnd = iSegEnd, iSegBegin
            
        # project point onto segment
        for iseg in  range(iSegBegin, iSegEnd+1):
            try:
                # @todo: , if the point does not lie on the current segment
                # the projection might not converge (there may be not solution).
                # this is not bad, but takes some time. Find a way to determine
                # in advance on which segment the points lies. one way to do so
                # is to project the segment edge on the intersection line and 
                # determine the intersection parameter
                (alpha, beta) = self.segments[iseg].projectPointOnCut(PL, P,N)
            except SegmentMathError:
                continue
            
            # in the last and first segment, alpha and beta dont have to be valid, due to the extension of the leading edge
            if SegmentGeometry.isValid(alpha, beta):
                return self.segments[iseg].getPoint(alpha, beta)[:,0]
            elif iseg == 0 and alpha < 0.:
                return self.segments[iseg].getPoint(alpha, beta)[:,0]
            elif iseg == nseg-1 and alpha > 1.:
                 return self.segments[iseg].getPoint(alpha, beta)[:,0]
        
        raise NameError('Error determining segment index in ComponentSegment.calcPoint')
        
    def project(self, point):
        
        # get the eta coordinate of the point
        eta = self.le.project(point[1:3])
        
        # get point on the leading edge and normal vector
        pyz, nyz, iSegBegin = self.le.calcPoint(eta)
        _ , _ , iSegEnd     = self.te.calcPoint(eta)
        P = array([0, pyz[0], pyz[1]])
        N = array([0, nyz[0], nyz[1]])
        
        # calculate intersection with leading edge of segment
        PV = self.segments[iSegBegin].calcIntersectLE(P,N)[:,0];
        PH = self.segments[iSegEnd].calcIntersectTE(P,N)[:,0];    
        
        # now project point back on the line pv-ph
        xsi = dot(point-PV, PH-PV)/(linalg.norm(PH-PV)**2)
        return eta, xsi
        
        
        
        
        
        

        
    def plot(self, axis=None):
        if not axis:
            axis = plt.gca(projection='3d')
        
        nseg = size(self.lepoints, 1) - 1
        style= 'b-'
        axis.plot(self.lepoints[0,:], self.lepoints[1,:], self.lepoints[2,:], style)
        axis.plot(self.tepoints[0,:], self.tepoints[1,:], self.tepoints[2,:], style)
        for iseg in xrange(0,nseg+1):
            axis.plot([self.lepoints[0,iseg], self.tepoints[0,iseg]], [self.lepoints[1,iseg], self.tepoints[1,iseg]], [self.lepoints[2,iseg], self.tepoints[2,iseg]], style)
        
        #calc iso-eta lines
        for eta in arange(0.,1.01, 0.1):
            xsis = linspace(0.,1.0, 20)
            P = zeros((3,size(xsis)))
            for i in xrange(0,size(xsis)):
                P[:,i] = self.calcPoint(eta, xsis[i])
           
            axis.plot(P[0,:], P[1,:], P[2,:],'r')
            
        #calc iso-xsi lines
        for xsi in arange(0.,1.01, 0.1):
            etas = linspace(0.,1.0, 20)
            P = zeros((3,size(etas)))
            for i in xrange(0,size(etas)):
                P[:,i] = self.calcPoint(etas[i], xsi)
           
            axis.plot(P[0,:], P[1,:], P[2,:],'r')
            
        axis.set_xlabel('x [m]')
        axis.set_ylabel('y [m]')
        axis.set_zlabel('z [m]')
        


vk = array([[ 0.0,  2.0, 3.4],
            [ 0.5,  4.4, 9.0], 
            [-0.3,  0.0, 0.4]])

hk = array([[3.5, 3.5, 4.0],
            [0.0, 6.4, 9.5], 
            [0.0, 0.0, 0.5]])

cs = ComponentSegment(vk, hk)
#print P


fig = plt.figure()

cs.plot()

P = cs.calcPoint(0.3, 0.7)
(eta, xsi)  = cs.project(P)
print 'eta,xsi:' , eta, xsi
fig.gca().plot([P[0]], [P[1]], [P[2]], 'gx')


plt.show()