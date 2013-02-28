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
# @file ms_optAlgs.py
# @brief Implementation of some basic optimization algorithms
#
import pkg_resources
pkg_resources.require("matplotlib")
from numpy import *
import matplotlib.pyplot as plt

def mb_backtrackingLineSearch(objFunc,objFuncValue,x,dx,direc):
	# 2010 m.bangert@dkfz.de
	# backtracking line search using armijo criterion
	#
	# objFunc      - handle for objective function
	# objFuncValue - current objective function value @ x
	# x            - x
	# dx           - dx
	# dir          - search direction
	#
	# example : mb_backtrackingLineSearch(objFunc,objFuncValue,x,dx,dir)

	alphaMax     = 1.; # this is the maximum step length
	alpha        = alphaMax;
	fac          = 1./2.; # < 1 reduction factor of alpha
	c_1          = 1e-1;
	while objFunc(x+alpha*direc) > objFuncValue + c_1*alpha*dot(direc,dx):
		alpha = fac*alpha;
		
		if alpha < 10*spacing(1):
			raise NameError('Error in Line search - alpha close to working precision');

	return alpha


def mb_quadraticApproximationLineSearch(objFunc,objFuncValue,x,dx,direc,alpha_in):
# 2010 m.bangert@dkfz.de
# line search using a quadratic approximation we are setting to minimum of
# the quadratic function phi(t) = a*t^2+b*t+c which is determined by three 
# points: objFunc(x), gradFunc'(x) and objFunc(x+alpha*dir)
#
# objFunc      - handle for objective function
# objFuncValue - current objective function value @ x
# x            - x
# dx           - dx
# dir          - search direction
#
# example : mb_backtrackingLineSearch(objFunc,objFuncValue,x,dx,dir)

	alpha = alpha_in;
	c     = objFuncValue;
	b     = dot(direc,dx)
	ofprime = objFunc(x+alpha*direc)
	a     = (ofprime  - b*alpha - c) / alpha**2;
	alpha = - b / (2*a);
	ofprime = objFunc(x+alpha*direc)

	
	numOfQuadApprox = 0;
	c_1             = 1e-1;
	# check if armijo criterion fulfilled
	while ofprime > objFuncValue + c_1*alpha*dot(direc,dx):
		numOfQuadApprox = numOfQuadApprox + 1;
		a = ( ofprime - b*alpha - c) / alpha**2;
		alpha = - b / (2*a);
		ofprime =objFunc(x+alpha*direc) 

		if numOfQuadApprox > 10:
			print 'Error in Line search - quadraric approximation failed more than 10 times\n Starting backtracking line search\n'
			return mb_backtrackingLineSearch(objFunc,objFuncValue,x,dx,direc)

	return alpha


def ms_optSteepestDescent(objFunc, gradFunc, x_start):
	return ms_optCG(objFunc, gradFunc, x_start, 'gradient')

# conjugate gradient, fletcher reeves	
def ms_optCG(objFunc, gradFunc, x_start, type='fr'):

	x               = x_start;
	oldx = x;
	objFuncValue    = objFunc(x);
	oldObjFuncValue = objFuncValue * 2.;
	alpha = 1.
	
	dx  = gradFunc(x);
	
	direc = -dx
	
	# iterate
	iteration   = 0;
	numOfIter = 100;
	prec      = 1e-7;
	
	# convergence if gradient smaller than prec, change in objective function
	# smaller than prec or maximum number of iteration reached...
	while (iteration < numOfIter) and (abs((oldObjFuncValue-objFuncValue)/max(1,objFuncValue))>prec) and (linalg.norm(dx)>prec):
		
		# iteration counter
		iteration = iteration + 1;
		alpha = mb_quadraticApproximationLineSearch(objFunc,objFuncValue,x,dx,direc,alpha);
		#alpha = mb_backtrackingLineSearch(objFunc,objFuncValue,x,dx,direc);
		
		# update x
		x = x + alpha*direc;
		
		plt.plot([x[0], oldx[0]], [x[1], oldx[1]],'r');
		
		oldx = x
		
		# update obj func values
		oldObjFuncValue = objFuncValue;
		objFuncValue    = objFunc(x);
		
		# update dx
		oldDx = dx;
		dx    = gradFunc(x);
		
		# fletcher reeves
		if type == 'fr':
			beta = dot(dx,dx)/dot(oldDx, oldDx)
		# hestenes stiefel
		elif type == 'hs':
			beta = dot(dx, dx-oldDx)/dot(direc, dx-oldDx)
		# polak rebiere
		elif type == 'pr':
			beta = dot(dx, dx-oldDx)/dot(oldDx, oldDx)
		else:
			beta = 0.
		
		# update search direction
		direc = -dx + direc * beta

		print 'Iter {}: of = {} @ x = {} alpha = {}.'.format(iteration, objFuncValue, x, alpha)

	return x	

def ms_optNewton(objFunc, gradFunc, hessFunc, x_start):

	x               = x_start;
	oldx = x;
	objFuncValue    = objFunc(x);
	oldObjFuncValue = objFuncValue * 2. + 1;
	
	dx  = gradFunc(x);
	hess = hessFunc(x);
	alpha = 1.
	
	direc = -linalg.solve(hess, dx)
	
	# iterate
	iteration   = 0;
	numOfIter = 100;
	prec      = 1e-7;
	
	# convergence if gradient smaller than prec, change in objective function
	# smaller than prec or maximum number of iteration reached...
	while (iteration < numOfIter) and (abs((oldObjFuncValue-objFuncValue)/max(1,objFuncValue))>prec) and (linalg.norm(dx)>prec):
		
		# iteration counter
		iteration = iteration + 1;
		if dot(dx,direc) >= 0:
			print 'Warning: hessian not positive definite. Go along gradient'
			direc = -dx
			
		alpha = mb_backtrackingLineSearch(objFunc,objFuncValue,x,dx,direc);
		
		
		# update x
		x = x + alpha*direc;
		
		plt.plot([x[0], oldx[0]], [x[1], oldx[1]],'r');
		
		oldx = x
		
		# update obj func values
		oldObjFuncValue = objFuncValue;
		objFuncValue    = objFunc(x);
		
		# update dx
		dx    = gradFunc(x);
		hess  = hessFunc(x);
		
		# update search direction
		direc = -linalg.solve(hess, dx)
		
		print 'Iter {}: of = {} @ x = {} alpha = {}.'.format(iteration, objFuncValue, x, alpha)

	return x


def ms_optQuasiNewton(objFunc, gradFunc, x_start, type='bfgs'):
	x               = x_start;
	oldx = x;
	objFuncValue    = objFunc(x);
	oldObjFuncValue = objFuncValue * 2.;
	
	dx  = gradFunc(x);
	hess = eye(x.size);
	
	direc = -linalg.solve(hess, dx)
	
	# iterate
	iteration   = 0;
	numOfIter = 100;
	prec      = 1e-7;
	alpha = 1.
	
	# convergence if gradient smaller than prec, change in objective function
	# smaller than prec or maximum number of iteration reached...
	while (iteration < numOfIter) and (abs((oldObjFuncValue-objFuncValue)/max(1,objFuncValue))>prec) and (linalg.norm(dx)>prec):
		
		# iteration counter
		iteration = iteration + 1;
		alpha = mb_backtrackingLineSearch(objFunc,objFuncValue,x,dx,direc);
		#alpha = mb_quadraticApproximationLineSearch(objFunc,objFuncValue,x,dx,direc,alpha);
		
		
		# update x
		p = alpha*direc
		x = x + p;
		
		plt.plot([x[0], oldx[0]], [x[1], oldx[1]],'r');
		
		oldx = x
		
		# update obj func values
		oldObjFuncValue = objFuncValue;
		objFuncValue    = objFunc(x);
		
		# update dx
		oldDx = dx;
		dx    = gradFunc(x);
		
		# calculate difference of gradients
		q  = dx-oldDx;
		
		# update hessian
		if type=='bfgs':
			hess = hess + outer(q,q)/dot(q,p) - outer(dot(hess,p), dot(hess,p))/dot(p, dot(hess, p))
		elif type=='sr1':
			hess = hess + outer(q-dot(hess,p), q-dot(hess,p))/dot(q-dot(hess,p),p);

		# update search direction
		direc = -linalg.solve(hess, dx)
		
		print 'Iter {}: of = {} @ x = {} alpha = {}.'.format(iteration, objFuncValue, x, alpha)

	return x
	
def ms_numGrad(objfun, x, h):
	fcur = objfun(x)
	dx = x * 0
	for i in range(0,size(x)):
		xnew = copy(x)
		xnew[i] = x[i] + h
		dx[i] = (objfun(xnew) - fcur)/h
	
	return dx

def ms_numHess(objGrad, x, h):
	curdx = objGrad(x)
	H = zeros((size(x), size(x)))
	for i in range(0,size(x)):
		xnew = copy(x)
		xnew[i] = x[i] + h
		H[i,:] = (objGrad(xnew) - curdx)/h
	
	return H
