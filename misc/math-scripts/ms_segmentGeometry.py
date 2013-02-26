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
# @file ms_segmentGeometry.py
# @brief Implementation of the segement geometry
#
# @todo Further implementation required
#

from numpy import *
from ms_optAlgs import *

import matplotlib as mpl
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt

class SegmentGeometry:
	def __init__(self, p1, p2, p3, p4):
		self.setPoints(p1, p2, p3, p4)
		
	def setPoints(self, p1, p2, p3, p4):
                self.a = -p1+p2;
                self.b = -p1+p3;
                self.c = p1-p2-p3+p4;
                self.d = p1;

                
        def calcPoint(self, alpha, beta):
            return outer(self.a,alpha) + outer(self.b,beta) + outer(self.c,alpha*beta) + outer(self.d,ones(size(alpha)));

        # calculates the tangents in eta and xsi direction at the given point
	def calcPointTangents(self, eta, xsi):
            J = zeros((3,2))
            J[:,0] = self.a + xsi*self.c
            J[:,1] = self.b + eta*self.c
            return J

        def calcOF(self, eta, xsi, x):
            p = self.calcPoint(eta,xsi);
            p = p[:,0]
            return dot(p-x,p-x)

        def calcGradient(self, eta, xsi, x):
            p = self.calcPoint(eta,xsi);
            p = p[:,0]
            J = self.calcPointTangents(eta,xsi);
            grad = zeros(2);
            grad[0] = 2*dot(J[:,0],p-x)
            grad[1] = 2*dot(J[:,1],p-x)
            return grad

        def calcHessian(self, eta, xsi, x):
            p = self.calcPoint(eta,xsi);
            p = p[:,0]
            J = self.calcPointTangents(eta,xsi);

            H = zeros((2,2))
            H[0,0] = 2*dot(J[:,0], J[:,0])
            H[1,1] = 2*dot(J[:,1], J[:,1])
            H[1,0] = 2*dot(J[:,1], J[:,0]) + 2*dot(p-x, self.c)
            H[0,1] = H[1,0] 
            
            return H

        def project(self, p):
            eta = dot(self.a,p-self.d)/dot(self.a,self.a)
            xsi = dot(self.b,p-self.d)/dot(self.b,self.b)
            x = array([eta,xsi])
            print 'initial guess: ', eta, xsi
            of = lambda ex: self.calcOF(ex[0], ex[1], p)
            ograd = lambda ex: self.calcGradient(ex[0], ex[1], p)
            ohess = lambda ex: self.calcHessian(ex[0], ex[1], p)

            fig2 = plt.figure();
		
	    X, Y = meshgrid(arange(-3, 4, 0.1), arange(-3, 4, 0.1))
	    Z = zeros(X.shape);
	
	    for i in range(0,size(X,0)):
                for j in range(0,size(X,1)):
                    Z[i,j] = of([X[i,j], Y[i,j]])
		
	    plt.imshow(Z,origin='lower', extent=[-3, 4,-3,4], aspect=1./1.)
	    plt.colorbar();	
	    plt.contour(X,Y,Z)
	    plt.title('Objective function')
	    plt.xlabel('eta');
	    plt.ylabel('xsi');

            x_= ms_optNewton(of,ograd,ohess,x)

	    eta = x_[0]; xsi = x_[1];
	    return (eta, xsi)
            

        def draw(self, axis):
	    for alpha in  arange(0,1.01,0.1):
                P1 = self.calcPoint(alpha, 0.)
                P2 = self.calcPoint(alpha, 1.)
		axis.plot([P1[0,0], P2[0,0]], [P1[1,0], P2[1,0]], [P1[2,0], P2[2,0]],'g');
				
	    for beta in arange(0,1.01,0.1):
                P1 = self.calcPoint(0, beta)
                P2 = self.calcPoint(1, beta)
		axis.plot([P1[0,0], P2[0,0]], [P1[1,0], P2[1,0]], [P1[2,0], P2[2,0]],'g');
            

