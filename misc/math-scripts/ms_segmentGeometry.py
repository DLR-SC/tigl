from numpy import *

def ms_calcSegmentPoint(x1,x2,x3,x4, alpha, beta):

	a = -x1+x2;
	b = -x1+x3;
	c = x1-x2-x3+x4;
	d = x1;

	return outer(a,alpha) + outer(b,beta) + outer(c,alpha*beta) + outer(d,ones(size(alpha)));

