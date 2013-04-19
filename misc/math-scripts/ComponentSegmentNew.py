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

from Polygonzug import PolygonNormal, PolygonWRoundedEdges
from ms_segmentGeometry import SegmentGeometry


from numpy import size,  array

class ComponentSegment:
    def __init__(self, le, te):
        assert(size(le,1) == size(te,1))
        assert(size(le,0) == size(te,0) == 3)
        
        self.segments = {}
        nseg = size(le,1)-1
        for iseg in range(0,nseg):
            self.segments[iseg] = SegmentGeometry(le[:,iseg], le[:,iseg+1], te[:,iseg], te[:,iseg+1])
        
        # project 
        # todo, extend leading edge according to cpacs specs
        self.le = PolygonWRoundedEdges(le[1:3,:])
        self.le.setRadius(0.01)
        self.te = PolygonNormal(te[1:3,:]) 
        
        
    def calcPoint(self, eta, xsi):
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
            if SegmentGeometry.isValid(alpha, beta):
                return self.segments[iseg].getPoint(alpha, beta)[:,0]
            
        
        raise NameError('Error determining segment index in ComponentSegment.calcPoint')

    


vk = array([[1, 0, 0, 0,0],
            [1, 2, 3, 4,3], 
            [1, 2, 1, 2,3]])

hk = array([[4, 3, 3, 3,3],
            [1, 2, 3, 4,3], 
            [1, 2, 1, 2,3]])

cs = ComponentSegment(vk, hk)
P = cs.calcPoint(0.25*0.5, 0.25)
print P

points = vk[1:3,:]


poly = PolygonNormal(points)



pz = PolygonWRoundedEdges(points)
pz.setRadius(0.2)

# 2 edges defined by 3 points
A = array([1, 1])
B = array([4, 4])
C = array([8, 2.5])

fig = plt.figure()
#fig.gca().plot([A[0], B[0], C[0]], [A[1], B[1], C[1]], 'r-')

pz.plot(fig.gca())

f = 0
while f <= 1.000:
    (P, N, iseg) = pz.calcPoint(f)
    fig.gca().plot(P[0],P[1], 'kx')
    fig.gca().plot([P[0], P[0]+ 0.15*N[0]],[P[1], P[1] + 0.15*N[1]], 'b')
    f = f + 1./32.




plt.show()