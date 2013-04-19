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

from numpy import array, outer, ones, size, dot, linalg, zeros, cross, transpose 
from ms_optAlgs import ms_optNewton

class SegmentGeometry:
	def __init__(self, p1, p2, p3, p4):
		self.setPoints(p1, p2, p3, p4)
		
	def setPoints(self, p1, p2, p3, p4):
		self.__a = -p1+p2;
		self.__b = -p1+p3;
		self.__c =  p1-p2-p3+p4;
		self.__d =  p1;

	def getPoint(self, alpha, beta):
		a = self.__a
		b = self.__b
		c = self.__c
		d = self.__d
		return outer(a,alpha) + outer(b,beta) + outer(c,alpha*beta) + outer(d,ones(size(alpha)));
	
	# calculates the intersection of the leading edge with a given plane
	def calcIntersectLE(self, p, n):
		eta = dot(p-self.__d,n)/ dot(self.__a,n)
		return self.getPoint(eta, 0.)

		# calculates the intersection of the trailing edge with a given plane
	def calcIntersectTE(self, p, n):
		eta = dot(p-self.__d - self.__b,n)/ dot(self.__c + self.__a,n)
		return self.getPoint(eta, 1.)
	
	# projects the point p on the cut of the segment with a plane (given by p, n)
	def projectPointOnCut(self, p_proj, p, n):
		debug = True

		# some constants for the intersection calculation
		a1 =  dot(p - self.__d, n);
		a2 = -dot(self.__b, n);
		a3 =  dot(self.__a, n);
		a4 =  dot(self.__c, n);

		# this calculates the intersection curve from the segment with a plane (normal vector n, point p2)
		al = lambda beta:  (a1 + a2*beta)/(a3 + a4*beta);
		# diff( eta(xi). xi), tangent in eta xsi space
		alp = lambda beta: (a2*a3 - a1*a4)/((a3 + a4*beta)**2);

		# 3d intersection curve, parameterized by beta [0,1]
		cu = lambda beta: outer(self.__a,al(beta)) + outer(self.__b,beta) + outer(self.__c,al(beta)*beta) + outer(self.__d,ones(size(beta)));
		# tangent in 3d space
		cup = lambda beta: (outer(self.__a,ones(size(beta))) + outer(self.__c,beta))*outer(ones(3),alp(beta)) + outer(self.__c,al(beta)) + outer(self.__b,ones(size(beta)));

		# calculate intersection points with leading and trailing edge of the segment
		pbeg = cu(0)[:,0]
		pend = cu(1)[:,0]
		reflen = linalg.norm(pbeg-pend);
		
		# project this point onto intersection curve i.e. find beta so that (cu(beta) - pact) * (pbeg-pend) == 0
		# as cu(beta) is not linear, we try to find the solution with Newton Raphson method
		f = lambda beta: dot(cu(beta)[:,0] - p_proj[:,0], pend - pbeg)/reflen
		fp = lambda beta: dot(cup(beta)[:,0], pend - pbeg)/reflen
		
		#initial guess
		beta =  0.5

		diff = f(beta)
		iter = 0;
		if debug: print 'Iter:', iter, ' Error=', abs(diff), ' @ Beta=' , beta
		while abs(diff) > 1e-12 and iter  < 20:
			iter += 1
			dir = -diff/(fp(beta))
			# maybe we need a line-search here...?
			beta = beta + dir
			diff = f(beta)
			if debug: print 'Iter:', iter, ' Error=', abs(diff), '@ Beta=' , beta
		
		if iter >= 20:
			print "ERROR: could not project intersection curve onto line"
			# set illegal value 
			beta = -1.
		
		
		return (al(beta), beta)
	
	def getTangents(self, alpha, beta):
		assert(size(alpha) == 1)
		assert(size(beta) == 1)
		
		J = zeros((3,2))
		J[:,0] = self.__a + beta*self.__c
		J[:,1] = self.__b + alpha*self.__c
		
		return J
		
	def getNormal(self, alpha, beta):
		J = self.getTangents(alpha, beta);
		normal = cross(J[:,1],J[:,0])
		return normal/linalg.norm(normal)
		
	# this function is NOT part of the segment API but is required for
	# the projection algorithm 
	def __calcHessian(self, alpha, beta, x):
		assert(size(alpha) == 1)
		assert(size(beta) == 1)
		
		hess = zeros((2,2));
		p = self.getPoint(alpha, beta)
		
		hess[0,0] = 2.*dot(self.__a+beta *self.__c, self.__a+beta *self.__c);
		hess[1,1] = 2.*dot(self.__b+alpha*self.__c, self.__b+alpha*self.__c);
		hess[1,0] = 2.*dot(self.__b+alpha*self.__c, self.__a+beta*self.__c) + 2.*dot((p-x)[:,0], self.__c);
		hess[0,1] = hess[1,0]
		
		return hess
		
		
	def projectOnSegment(self, p):
		of = lambda x: linalg.norm(self.getPoint(x[0], x[1]) - p)**2
		ograd = lambda x: 2.*dot(transpose(self.getTangents(x[0], x[1])),self.getPoint(x[0], x[1]) - p)[:,0]
		ohess = lambda x: self.__calcHessian(x[0],x[1], p)
		# numerical gradient and hessian, just for error checking
		# ograd_num = lambda x: ms_numGrad(of, x, 1e-8)
		# ohess_num = lambda x: ms_numHess(ograd, x, 1e-8)
		
		# initial guess, could be improved but works
		x = array([0., 0.]);
		x = ms_optNewton(of, ograd, ohess, x)
		
		eta = x[0]
		xsi = x[1]
		
		return eta,xsi
	
	@staticmethod
	def isValid(eta, xsi): 
		if eta >= 0. and eta <= 1. and xsi >= 0. and xsi <= 1.:
			return True
		else:
			return False
		
