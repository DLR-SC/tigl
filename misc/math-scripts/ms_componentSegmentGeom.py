#
# Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
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
# @file ms_componentSegmentGeom.py
# @brief Implements coordinate transforms on the component segment geometry
#

from numpy import *
from ms_optAlgs import *
from ms_segmentGeometry import *

import matplotlib as mpl
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt

class Elongation:
	Left, No, Right, LeftRight = range(4)

class ComponentSegment:
	
	def __init__(self):
		self.segments = []

	def addSegment(self, p1, p2, p3, p4):
		self.segments.append(ComponentSegmentGeometry(p1,p2,p3,p4))
		self.__calcEtaRanges()
		
	def __calcEtaRanges(self):
		size = len(self.segments)
		# calculate total projected length
		len_tot = 0
		for item in self.segments:
			len_tot = len_tot + item.calcProjectedLeadingEdgeLength(Elongation.No)
		
		len_tot = len_tot + self.segments[0].calcProjectedLeadingEdgeLength(Elongation.Left) \
						- self.segments[0].calcProjectedLeadingEdgeLength(Elongation.No) \
						+ self.segments[size-1].calcProjectedLeadingEdgeLength(Elongation.Right) \
						- self.segments[size-1].calcProjectedLeadingEdgeLength(Elongation.No)
				
		#calculate inner eta of first segment
		etastart = (self.segments[0].calcProjectedLeadingEdgeLength(Elongation.Left) \
					- self.segments[0].calcProjectedLeadingEdgeLength(Elongation.No) )/len_tot
		
		for item in self.segments:
			etastop = etastart + item.calcProjectedLeadingEdgeLength(Elongation.No)/len_tot
			item.setLeadingEdgeEtas(etastart, etastop)
			etastart = etastop


	def draw(self, axis):
		for item in self.segments:
			item.drawSegment(axis)
			
	def calcPoint(self, eta, xsi):
		for item in self.segments:
			if item.checkCoordValidity(eta,xsi) == True:
				return item.calcCSPoint(eta,xsi)

	def drawPoint(self, axis, eta, xsi):
		point = self.calcPoint(eta, xsi)
		axis.plot([point[0]], [point[1]], [point[2]],'rx')


class ComponentSegmentGeometry:
	def __init__(self, p1, p2, p3, p4, etamin = 0, etamax = 1):
		self.setPoints(p1, p2, p3, p4, etamin, etamax)
		
	def setPoints(self, p1, p2, p3, p4, etamin, etamax):
		self.__p1 = p1
		self.__p2 = p2
		self.__p3 = p3
		self.__p4 = p4
		self.__etamin = etamin
		self.__etamax = etamax

		sv = p2 - p1
		sh = p4 - p3
		
		# normal vector of plane
		n = array([0, -sv[1], -sv[2]])
		
		## calculate extended leading edge and trailing edge points, this has to be done
		#  only once per wing segment. 
		
		#calculate point where le intersects plane
		avo = dot(p4-p1,n) / dot(p2-p1,n)
		if avo > 1:
			self.__p2p = avo*sv + p1
			self.__p4p = p4
		else:
			self.__p2p = p2;
			# check trailing edge
			aho = dot(p2-p3,n) / dot(p4-p3,n)
			assert aho >= 1
			self.__p4p = sh*aho + p3
	
		# now the inner section, the normal vector should be still the same
		avi = dot(p3-p1,n)/dot(p2-p1,n)
		if avi < 0:
			# leading edge has to be extended
			self.__p3p = p3
			self.__p1p = avi*sv + p1
		else:
			self.__p1p = p1;
			ahi = dot(p1-p3,n)/dot(p4-p3,n)
			self.__p3p = p3 + ahi*sh
			assert ahi <= 0
			
		#calculate eta values of segment edges, these values define also, when a given cs coordinate is outside the wing segment
		self.__eta1 = dot(p1-self.__p1p,n) / dot(self.__p2p-self.__p1p,n)
		self.__eta2 = dot(p2-self.__p1p,n) / dot(self.__p2p-self.__p1p,n)
		self.__eta3 = dot(p3-self.__p1p,n) / dot(self.__p2p-self.__p1p,n)
		self.__eta4 = dot(p4-self.__p1p,n) / dot(self.__p2p-self.__p1p,n)

	# sets the eta range from inner segment tip to the outer tip
	def setEtaMinMax(self, etamin, etamax):
		self.__etamax = etamax
		self.__etamin = etamin
		
	def getEtaMinMax(self):
		return self.__etamin, self.__etamax
	
	# sets the eta range of the leading edge. if e.g. the trailing edge is longer than
	# the leading edge, this makes a difference to setEtaMinMax
	def setLeadingEdgeEtas(self, eta_in, eta_out):
		# we need to scale etamax, etamin accordingly
		etamin = (self.__eta2*eta_in - self.__eta1*eta_out)/(self.__eta2 - self.__eta1)
		etamax = etamin + (eta_out - eta_in)/(self.__eta2 - self.__eta1)
		self.setEtaMinMax(etamin, etamax)

	# only valid with calcsCSPoint (not calcCSPoint2/3)
	def checkCoordValidity(self, eta, xsi):
		if eta < self.__etamin or eta > self.__etamax or xsi < 0 or xsi > 1:
			return False
		else:
			actetamin = (1-xsi)*self.__eta1 + xsi*self.__eta3
			actetamax = (1-xsi)*self.__eta2 + xsi*self.__eta4
			if (eta>= actetamin) and (eta <= actetamax):
				return True
			else:
				return False
			
		
	def calcProjectedLeadingEdgeLength(self, elongation):
		p1 = self.__p1;
		p2 = self.__p2;
		if elongation   == Elongation.Left:
			p1 = self.__p1p
		elif elongation == Elongation.Right:
			p2 = self.__p2p
		elif elongation == Elongation.LeftRight:
			p1 = self.__p1p
			p2 = self.__p2p
		
		# project leading edge into the z-y plane
		vProj = array([0, 1, 1])
		return linalg.norm(vProj*(p2-p1))
		
	def calcCSPoint(self, eta, xsi):
		eta_ = (eta - self.__etamin)/(self.__etamax - self.__etamin)
	
		#calculate eta values at given xsi 
		eta1p = self.__eta1*(1-xsi) + self.__eta3*xsi
		eta2p = self.__eta2*(1-xsi) + self.__eta4*xsi
		
		pbeg = self.__p1*(1-xsi) + self.__p3*xsi
		pend = self.__p2*(1-xsi) + self.__p4*xsi
		
		p = pbeg + (pend-pbeg)*(eta_ - eta1p)/(eta2p-eta1p)
		
		return p

	def calcCSPoint2(self, eta, xsi):
		eta_ = (eta - self.__etamin)/(self.__etamax - self.__etamin)
		
		p = self.__p1p + (self.__p2p-self.__p1p)*(eta_);
		
		a = -self.__p1+self.__p2;
		b = -self.__p1+self.__p3;
		c =  self.__p1-self.__p2-self.__p3+self.__p4;
		d =  self.__p1;
		
		n = array([0, -a[1], -a[2]])
		
		# calc some constants
		a1 =  dot(p - d, n);
		a2 = -dot(b, n);
		a3 =  dot(a, n);
		a4 =  dot(c, n);


		# this calculates the intersection curve from the segment with a plane (normal vector n, point p2)
		al = lambda beta:  (a1 + a2*beta)/(a3 + a4*beta);
		# diff( eta(xi). xi), tangent in eta xsi space
		alp = lambda beta: (a2*a3 - a1*a4)/((a3 + a4*beta)**2);

		# 3d intersection curve, parametrized by beta [0,1]
		cu = lambda beta: outer(a,al(beta)) + outer(b,beta) + outer(c,al(beta)*beta) + outer(d,ones(size(beta)));

		# tangent in 3d space
		cup = lambda beta: (outer(a,ones(size(beta))) + outer(c,beta))*outer(ones(3),alp(beta)) + outer(c,al(beta)) + outer(b,ones(size(beta)));

		#norm of tangent curve
		f =  lambda beta: sqrt(sum(cup(beta)**2.,0));

		# we want to integrate int f(x)*dx, we do gaussian method

		# substitution of f to range [-1,1]
		g = lambda x,beta: beta/2.*f((x+1.)*beta/2.);
		
		# gauss x points 5th order, this is really some dark magic ;)
		x = array([9.06179845938664e-01,
				5.38469310105683e-01,
				0.00000000000000e+00,
				-5.38469310105683e-01,
				-9.06179845938664e-01])
		
		# gauss weights
		w = array([2.36926885056189e-01,
				4.78628670499366e-01,
				5.68888888888889e-01,
				4.78628670499366e-01,
				2.36926885056189e-01])
		
		# calculate total length of iso eta curve
		ltot =  dot(g(x,1.),w);
		
		#now we use Newton Raphson to find beta, such that F(beta) == xi*ltot
		F = lambda beta: dot(g(x,beta),w)/ltot - xsi
		beta = xsi
		diff = F(beta)
		while abs(diff) > 1e-12:
			dir = -diff/(f(beta)/ltot)
			beta = beta + dir
			diff = F(beta)
			
		return cu(beta)
		
		
	# alternative implementation, where xsi resembles the relative coordinate
	# between intersection point of the leading edge and the intersection point
	# of the trailing edge. This intermediate point will then be projected onto
	# the true intersection curve
	def calcCSPoint3(self, eta, xsi):
		debug = True
		
		eta_ = (eta - self.__etamin)/(self.__etamax - self.__etamin)
		
		p = self.__p1p + (self.__p2p-self.__p1p)*(eta_);
		
		
		a = -self.__p1+self.__p2;
		b = -self.__p1+self.__p3;
		c =  self.__p1-self.__p2-self.__p3+self.__p4;
		d =  self.__p1;
		
		n = array([0, -a[1], -a[2]])
		
		# calc some constants
		a1 =  dot(p - d, n);
		a2 = -dot(b, n);
		a3 =  dot(a, n);
		a4 =  dot(c, n);


		# this calculates the intersection curve from the segment with a plane (normal vector n, point p2)
		al = lambda beta:  (a1 + a2*beta)/(a3 + a4*beta);
		# diff( eta(xi). xi), tangent in eta xsi space
		alp = lambda beta: (a2*a3 - a1*a4)/((a3 + a4*beta)**2);

		# 3d intersection curve, parametrized by beta [0,1]
		cu = lambda beta: outer(a,al(beta)) + outer(b,beta) + outer(c,al(beta)*beta) + outer(d,ones(size(beta)));

		# tangent in 3d space
		cup = lambda beta: (outer(a,ones(size(beta))) + outer(c,beta))*outer(ones(3),alp(beta)) + outer(c,al(beta)) + outer(b,ones(size(beta)));

		# calculate intersection with leading and trailing edge
		pbeg = cu(0)[:,0]
		pend = cu(1)[:,0]
		reflen = linalg.norm(pbeg-pend);
		
		# go along this line
		pact = (1-xsi)*pbeg + xsi*pend
		
		# project this point onto intersection curve i.e. find beta so that (cu(beta) - pact) * (pbeg-pend) == 0
		# as cu(beta) is not linear, we try to find the solution with newton raphson method
		f = lambda beta: dot(cu(beta)[:,0] - pact, pend - pbeg)/reflen
		fp = lambda beta: dot(cup(beta)[:,0], pend - pbeg)/reflen
		
		beta =  xsi

		diff = f(beta)
		iter = 0;
		if debug: print 'Iter:', iter, ' Error=', abs(diff), ' @ Beta=' , beta
		while abs(diff) > 1e-12 and iter  < 20:
			iter += 1
			dir = -diff/(fp(beta))
			# maybe we need a linesearch here...
			beta = beta + dir
			diff = f(beta)
			if debug: print 'Iter:', iter, ' Error=', abs(diff), '@ Beta=' , beta
		
		if iter >= 20:
			print "ERROR: could not project intersection curve onto line"
			
			if debug == True:
				myb = linspace(-1.,1., 1000)
				val = 0*myb
				for i in range(len(myb)):
					val[i] = f(myb[i]);
				
				fig = plt.figure()
				ax2 = fig.gca()
				ax2.plot(myb, val)
		
		# calculate result
		point = cu(beta)
		
		# here we got for free our segment coordinates also, which are
		# eta_s = al(beta), xsi_s = beta 
		
		return point
		

	# calculates the tangents in eta and xsi direction at the given point
	def calcCSPointTangents(self, eta, xsi):
		eta_ = (eta - self.__etamin)/(self.__etamax - self.__etamin)
		deta_ = 1. /(self.__etamax - self.__etamin)

		#calculate eta values at given xsi 
		eta1p = self.__eta1*(1-xsi) + self.__eta3*xsi
		eta2p = self.__eta2*(1-xsi) + self.__eta4*xsi
		
		pbeg = self.__p1*(1-xsi) + self.__p3*xsi
		pend = self.__p2*(1-xsi) + self.__p4*xsi
	
		# calculate derivatives
		deta1p = self.__eta3 - self.__eta1;
		deta2p = self.__eta4 - self.__eta2;
		dpbeg  = self.__p3   - self.__p1;
		dpend  = self.__p4   - self.__p2;
		
		J = zeros((3,2))
		J[:,0] = (pend-pbeg)/(eta2p-eta1p)*deta_;
		J[:,1] = dpbeg + (dpend-dpbeg)*(eta_ - eta1p)/(eta2p-eta1p) + (pend - pbeg)*(-deta1p/(eta2p-eta1p) - (eta_ - eta1p)/((eta2p-eta1p)**2)*(deta2p - deta1p) );
		
		return J


	def calcCSPointNormal(self,  eta, xsi):
		J = self.calcCSPointTangents(eta, xsi);
		normal = cross(J[:,1],J[:,0])
		return normal/linalg.norm(normal)


	def __calcCSHessian(self, eta, xsi, p):
		eta_ = (eta - self.__etamin)/(self.__etamax - self.__etamin)
		deta_ = 1. /(self.__etamax - self.__etamin)
		
		#calculate eta values at given xsi 
		eta1p = self.__eta1*(1-xsi) + self.__eta3*xsi
		eta2p = self.__eta2*(1-xsi) + self.__eta4*xsi
		
		pbeg = self.__p1*(1-xsi) + self.__p3*xsi
		pend = self.__p2*(1-xsi) + self.__p4*xsi
	
		# calculate derivatives
		deta1p = self.__eta3 - self.__eta1;
		deta2p = self.__eta4 - self.__eta2;
		dpbeg = self.__p3 - self.__p1;
		dpend = self.__p4 - self.__p2;
	
	
		# helper variables and their derivatives to xsi
		hv1 =  pend -  pbeg;
		dhv1 = dpend - dpbeg;
		
		h2 = eta_ -  eta1p;
		dh2 =    - deta1p;
		
		h3 =  1/(eta2p-eta1p);
		dh3 = -1/(eta2p-eta1p)**2*(deta2p-deta1p);
		d2h3 = 2/(eta2p-eta1p)**3*(deta2p-deta1p)**2;
		
		# p(eta, xsi)
		p_ = pbeg + hv1*h2*h3;
		
		# first derivative, dp(eta, xsi)
		J1 = hv1*h3*deta_;
		J2 = dpbeg + dhv1*h2*h3 + hv1*(dh2*h3 + h2 * dh3);
		
		# second order derivative d2p(eta, xsi), H11 is zero!
		H21 =   (dhv1*h3 + hv1*dh3)*deta_;
		H22 =   dhv1*(dh2*h3 + h2*dh3)*2 + hv1*( 2*dh2*dh3 + h2*d2h3 );
		
		# finally applying for the object function
		H = zeros((2,2))
		H[0,0] = dot(J1,J1)
		H[0,1] = dot(J1,J2) + dot(p_ - p, H21)
		H[1,1] = dot(J2,J2) + dot(p_ - p, H22)
		H[1,0] = H[0,1];
		
		return 2.*H
	
	def projectOnCS(self, p):
		opttype = 'newton'
		# calculate initial guess, project onto leading edge and inner section
		eta = dot(p - self.__p1p, self.__p2p - self.__p1p)/( linalg.norm(self.__p2p - self.__p1p)**2)
		xsi = dot(p - self.__p1, self.__p3 - self.__p1)/( linalg.norm(self.__p3 - self.__p1)**2)
		# scale according to local eta range
		eta = eta*(self.__etamax-self.__etamin) + self.__etamin
		x = array([eta,xsi])
		
		of    = lambda x: linalg.norm(self.calcCSPoint(x[0], x[1])-p)**2;
		ograd = lambda x: 2.* dot(self.calcCSPointTangents(x[0], x[1]).transpose(), self.calcCSPoint(x[0], x[1])-p);
		#ograd = lambda x: ms_numGrad(of, x, 1e-9)
		ohess = lambda x: self.__calcCSHessian( x[0], x[1], p);
		#ohess = lambda x:  ms_numHess(ograd, x, 1e-9)
		
		fig2 = plt.figure();
		
		X, Y = meshgrid(arange(self.__etamin-0.2, self.__etamax+0.2, 0.02), arange(-0.2, 1.2, 0.02))
		Z = zeros(X.shape);
	
		for i in range(0,size(X,0)):
			for j in range(0,size(X,1)):
				Z[i,j] = of([X[i,j], Y[i,j]])
		
		plt.imshow(Z,origin='lower', extent=[self.__etamin-0.2, self.__etamax+0.2,-0.2,1.2], aspect=1./1.)
		plt.colorbar();	
		plt.contour(X,Y,Z)
		plt.title('Objective function opt:'+opttype)
		plt.xlabel('eta');
		plt.ylabel('xsi');
	
		if opttype == 'bfgs':
			x_= ms_optQuasiNewton(of,ograd, x, 'bfgs')
		elif opttype == 'sr1':
			x_= ms_optQuasiNewton(of,ograd, x, 'sr1')
		elif opttype == 'gradient':
			x_= ms_optSteepestDescent(of,ograd, x)
		elif opttype == 'cg':
			x_= ms_optCG(of,ograd, x, 'fr')
		else:
			x_= ms_optNewton(of,ograd,ohess,x)
		
		eta = x_[0]; xsi = x_[1];
		return (eta, xsi)
	
	def projectOnCS3(self, p):
		segment = SegmentGeometry(self.__p1, self.__p2, self.__p3, self.__p4)
		# get the projection point on the, here we get some numerical uncertainty
		# if we'd knew that p is already on the plane, we could directly use p 
		alpha, beta = segment.projectOnSegment(p);
		return self.convertAlphaBetaToEtaXsi(alpha, beta)
		#return self.convertXYZtoEtaXsi(p[:,0])
		

		
	def convertAlphaBetaToEtaXsi(self, alpha, beta):
		segment = SegmentGeometry(self.__p1, self.__p2, self.__p3, self.__p4)
		p_proj = segment.getPoint(alpha, beta)[:,0]
		return self.convertXYZtoEtaXsi(p_proj)

	# we must ensure that p_proj already lies on the segment, if unsure use projectOnCS3
	def convertXYZtoEtaXsi(self, p_proj):
		# project leading edge into the z-y plane
		vProj = array([0, 1, 1])
		n = vProj*(self.__p2p-self.__p1p)
		
		# calc eta koordinate of that point
		eta = (dot(p_proj,n)*(self.__eta2 - self.__eta1) + dot(self.__p2,n)*self.__eta1 - dot(self.__p1,n)*self.__eta2) \
				/ dot(self.__p2 - self.__p1, n);
		
		# intersection point of plane with leading edge
		p_ = 1./(self.__eta2 - self.__eta1)*( (self.__eta2 - eta)*self.__p1 + (eta - self.__eta1)*self.__p2 )
		
		a = -self.__p1+self.__p2;
		b = -self.__p1+self.__p3;
		c =  self.__p1-self.__p2-self.__p3+self.__p4;
		d =  self.__p1;
		# calc some constants
		a1 =  dot(p_ - d, n);
		a2 = -dot(b, n);
		a3 =  dot(a, n);
		a4 =  dot(c, n);
		
		# this calculates the intersection curve from the segment with a plane (normal vector n, point p2)
		al = lambda beta:  (a1 + a2*beta)/(a3 + a4*beta);
		# 3d intersection curve, parametrized by beta [0,1]
		cu = lambda beta: outer(a,al(beta)) + outer(b,beta) + outer(c,al(beta)*beta) + outer(d,ones(size(beta)));
		
		# now we have to find the xi coordinate, i.e. (pbeg + (pend-pbeg)*xi - p_proj)*(pbeg-pend) == 0
		pbeg = cu(0)[:,0]
		pend = cu(1)[:,0]
		
		xsi = dot(p_proj - pbeg, pbeg-pend)/dot(pend-pbeg, pbeg-pend);
		
		return eta, xsi

	def calcCSIsoXsiLine(self, xsi, extentToGeometry = False):
		etamin = self.__etamin
		etamax = self.__etamax
		
		if not extentToGeometry:
			etamin = xsi * (self.__eta3 - self.__eta1) + self.__eta1
			etamin = etamin * (self.__etamax - self.__etamin) + self.__etamin
			etamax = xsi * (self.__eta4 - self.__eta2) + self.__eta2
			etamax = etamax * (self.__etamax - self.__etamin) + self.__etamin
		
		P1 = self.calcCSPoint(etamin,xsi);
		P2 = self.calcCSPoint(etamax,xsi);
		return ([P1[0], P2[0]],  [P1[1], P2[1]], [P1[2], P2[2]] )


	def calcCSIsoEtaLine(self, eta, extentToGeometry = False, npoints = 30):
		eta_ = (eta - self.__etamin)/(self.__etamax - self.__etamin)
	
		# calculate bilinear vectors
		a = -self.__p1 + self.__p2;
		b = -self.__p1 + self.__p3;
		c =  self.__p1 - self.__p2 - self.__p3 + self.__p4;
		d =  self.__p1;
	
		# leading edge vector
		sv = self.__p2 - self.__p1
		# normal vector of intersection plane
		n = array([0, -sv[1], -sv[2]])
	
		# calculate eta point on leading edge
		p_ = self.__p1p*(1-eta_) + eta_*self.__p2p;

		a1 =  dot(p_-d,n);
		a2 = -dot(b,n);
		a3 =  dot(a,n);
		a4 =  dot(c,n); 
	
		# this calculates the intersection curve from the segment with a plane (normal vector n, point p2)
		al = lambda beta: (a1 + a2*beta)/(a3 + a4*beta);
	
		# 3d intersection curve, parameterized by beta [0,1]
		cu = lambda beta: outer(a,al(beta)) + outer(b,beta) + outer(c,al(beta)*beta) + outer(d, ones(size(beta)));
	
		xsistart = 0 if (eta_ <= self.__eta2) else (eta_ - self.__eta2)/(self.__eta4 - self.__eta2)
		xsistop  = 1 if (eta_ <= self.__eta4) else (eta_ - self.__eta2)/(self.__eta4 - self.__eta2)
		
		xsistart = xsistart if (eta_ >= self.__eta1) else (eta_ - self.__eta1)/(self.__eta3 - self.__eta1)
		xsistop  = xsistop  if (eta_ >= self.__eta3) else (eta_ - self.__eta1)/(self.__eta3 - self.__eta1)
	
		if extentToGeometry:
			xsi = linspace(0,1,npoints)
		else:
			xsi = linspace(xsistart,xsistop,npoints)
		points = cu(xsi);
	
		X = points[0,:];
		Y = points[1,:];
		Z = points[2,:];
	
		return (X,Y,Z)

	def drawSegment(self, axis, extentToGeometry = False):
		start  = math.ceil (self.__etamin*10.)/10.
		stop =   math.floor(self.__etamax*10.)/10.
		
		alpha = start
		while alpha <= stop:
				X,Y,Z = self.calcCSIsoEtaLine(alpha, extentToGeometry)
				axis.plot(X,Y,Z,'g')
				alpha = alpha + 0.1
				
		beta = 0.0
		while beta <= 1.0:
			X,Y,Z = self.calcCSIsoXsiLine(beta, extentToGeometry)
			axis.plot(X,Y,Z,'g');
			beta = beta + 0.1
			
		lw = 2
		axis.plot([self.__p1[0], self.__p2[0]], [self.__p1[1], self.__p2[1]], [self.__p1[2], self.__p2[2]],'b',linewidth=lw);
		axis.plot([self.__p1[0], self.__p3[0]], [self.__p1[1], self.__p3[1]], [self.__p1[2], self.__p3[2]],'b',linewidth=lw);
		axis.plot([self.__p4[0], self.__p2[0]], [self.__p4[1], self.__p2[1]], [self.__p4[2], self.__p2[2]],'b',linewidth=lw);
		axis.plot([self.__p3[0], self.__p4[0]], [self.__p3[1], self.__p4[1]], [self.__p3[2], self.__p4[2]],'b',linewidth=lw);
