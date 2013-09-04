#import matplotlib as mpl
#from mpl_toolkits.mplot3d import Axes3D

from math import atan

from numpy import array, linalg, dot, arccos, cos, sin, linspace, pi, size, arange, zeros,cumsum, diff, sqrt

def cart2pol(x, y):
    r = sqrt(x*x + y*y)
    if x == 0 and y >= 0:
        phi = pi/2
    elif x == 0 and y < 0:
        phi = 3/2*pi
    elif x > 0 and y >= 0:
        phi = atan(y/x)
    elif x < 0 and y >= 0:
        phi = pi + atan(y/x)
    elif x < 0 and y < 0:
        phi = pi + atan(y/x)
    elif x > 0 and y < 0:
        phi = 2*pi + atan(y/x)
    return (phi, r)

def pol2cart(phi,r):
    x = r*cos(phi)
    y = r*sin(phi)
    return array([x, y])

class PolygonWRoundedEdges(object):
    
    def __init__(self, points):
        self.points = points
        self.r = 0.3        
        self.midpoints = None
        self.alphasOfCircleCut = None
        self.angles = None
        
        self.calcInternalData()
        
    def setRadius(self, rad):
        self.r = rad
        self.calcInternalData()
        
    def calcInternalData(self):
        points= self.points;
        nlines = size(self.points,1)-1
        nmidpoints = size(self.points,1)-2
        
        self.midpoints = zeros([2, nmidpoints])
        self.angles = zeros([2, nmidpoints])
        self.alphasOfCircleCut = zeros([2, nlines])
        self.alphasOfCircleCut[0,0]        = 0
        self.alphasOfCircleCut[1,nlines-1] = 1
        
        # for each circular segment, compute center and angles
        # for each line segment, calculate local coordinates of points, where a circular segment begins/ends
        for i in arange(0,nmidpoints):
            A = points[:,i]
            B = points[:,i+1]
            C = points[:,i+2]
            r = self.r
            # calculate unit vectors
            e1 = (B-A)/linalg.norm(B-A)
            e2 = (C-B)/linalg.norm(C-B)
            
            alpha = arccos(dot(e1,e2))
            
            # no real angle
            if abs(alpha) < 1e-7 or abs(alpha - pi) < 1e-7:
                self.midpoints[:,i] = B
                self.alphasOfCircleCut[1,i]   = 1
                self.alphasOfCircleCut[0,i+1] = 0
                
                self.angles[0,i] = 0
                self.angles[1,i] = 0
            # true angle    
            else:
                # axis of mid-point
                ax = e2 - e1
                ax = ax /linalg.norm(ax)

                M = B + ax*r/cos(alpha/2);
                self.midpoints[:,i] = M
            
                # calculate relative coordinates, where tangents meet the segment
                s1 = dot(M-A,e1);
                s2 = dot(M-B,e2);
                self.alphasOfCircleCut[1,i]   = s1/linalg.norm(B-A);
                self.alphasOfCircleCut[0,i+1] = s2/linalg.norm(C-B);

                # calculate intersection points and corresponding angles
                P1 = A + s1*e1;
                P2 = B + s2*e2;
                psta = cart2pol(P1[0]-M[0], P1[1]-M[1])[0]
                psto = cart2pol(P2[0]-M[0], P2[1]-M[1])[0]
            
                # always chose an angle between 0 and pi between 2 edges
                if psta - psto > pi:
                    psto = psto + 2*pi
                elif psto - psta > pi:
                    psta = psta + 2*pi            
            
                self.angles[0,i] = psta
                self.angles[1,i] = psto
            
        partlens = zeros(2*nlines)
        for i in arange(0,nlines):
            A = points[:,i]
            B = points[:,i+1]
            partlens[2*i+1] = linalg.norm(B-A)*(self.alphasOfCircleCut[1,i]- self.alphasOfCircleCut[0,i])
            
        for i in arange(0,nmidpoints):
            A = points[:,i]
            B = points[:,i+1]
            C = points[:,i+2]
            partlens[2*i+2] = linalg.norm(B-A)*(1-self.alphasOfCircleCut[1,i]) + linalg.norm(C-B)*(self.alphasOfCircleCut[0,i+1])

        total_len = sum(partlens);
            
        self.etas =  cumsum(partlens)/total_len

    # calculates for a given eta the point on the polygon, the normal vector and the segment index
    def calcPoint(self, eta):
        eps = 1e-7
        assert( eta > -eps and eta <= 1+eps)
        
        neta = size(self.etas);
        
        assert(neta >= 2)
        
        # find index with etas[i] < eta < etas[i+1]
        ifound = 0
        for i in arange(1,neta):
            if self.etas[i] >= eta:
                ifound = i
                break
            
        if eta > self.etas[neta-1]:
            ifound = neta-1
        elif eta < 0:
            ifound = 1
        
        etastop = self.etas[ifound]
        etastart = self.etas[ifound-1]    
        eta_seg = (eta-etastart)/(etastop-etastart)    
        
        # if ifound even -> circular segment, else linear segment
        if ifound % 2 == 0:
            icirc = ifound/2-1
            angle = self.angles[0,icirc]*(1-eta_seg) + self.angles[1,icirc]*eta_seg

            # determine direction of circle
            if self.angles[0,icirc] < self.angles[1,icirc]:
                N = pol2cart(angle + pi/2, 1)
            else:
                N = pol2cart(angle - pi/2, 1)
             
            # calc the point in the line segments
            # determine, which segment   
            if eta_seg <= 0.5:
                iseg = icirc
                etastop = self.etas[ifound-1]
                etastart = self.etas[ifound-2]    
            else:
                iseg = icirc + 1
                etastop = self.etas[ifound+1]
                etastart = self.etas[ifound]  
            
            eta_seg = (eta-etastart)/(etastop-etastart)   
          
        else:
            iseg = (ifound-1)/2
            N = self.points[:,iseg+1] - self.points[:,iseg]
            N = N/linalg.norm(N)
            
        seg_loc = self.alphasOfCircleCut[0,iseg]*(1-eta_seg) + self.alphasOfCircleCut[1,iseg]*eta_seg;
        P = self.points[:,iseg]*(1-seg_loc) + self.points[:,iseg+1]*seg_loc;

            
        return (P,N, iseg)
        
    def project(self, point):
        '''
        Projects the point onto the polyline.
        Returns line parameter eta of projection.
        
        If multiple projections are possiblie, the projection point
        with the smallest distance to original point is used
        '''
        nlines = size(self.points,1)-1
        
        dists = [0.]*nlines
        etas =  [0.]*nlines
        for iseg in xrange(nlines):
            # line segment
            B = self.points[:,iseg+1]
            A = self.points[:,iseg]
            
            # calc local alpha
            alpha = dot(point-A, B-A)/(linalg.norm(B-A)**2)
            
            # calc dist
            p_proj = A + (B-A)*alpha
            dists[iseg] = linalg.norm(point-p_proj)            
            
            alpha1 = self.alphasOfCircleCut[0,iseg]
            alpha2 = self.alphasOfCircleCut[1,iseg]
            eta1   = self.etas[iseg*2]
            eta2   = self.etas[iseg*2+1]
            eta = (eta2-eta1)/(alpha2-alpha1)*(alpha-alpha1) + eta1
            etas[iseg] = eta
        
        min_index = dists.index(min(dists))
        return etas[min_index]
            

            
        
    def plot(self, axis):
        nlines = size(self.points,1)-1
        points = self.points
        axis.hold(True)
        for i in arange(0,nlines):
            axis.plot([points[0,i], points[0,i+1]], [points[1,i], points[1,i+1]],'r')
        
        for i in arange(0, nlines-1):
            M = self.midpoints[:,i]
            axis.plot(M[0],M[1],'mx')
            phi = linspace(self.angles[0,i],self.angles[1,i], 100)

            X = self.r*cos(phi)+ M[0];
            Y = self.r*sin(phi)+ M[1];
            axis.plot(X,Y,'g') 

 
class PolygonNormal(object):
    def __init__(self, points):
        self.points = points
        self.calcInternalData()
        
    def calcInternalData(self):
        points = self.points
        
        partlens = sqrt(sum(diff(points,1)**2,0))
        total_len = sum(partlens)
        self.alphasOfCircleCut = zeros(size(partlens)+1);
        self.alphasOfCircleCut[1:] = cumsum(partlens)/total_len
        
    # calculates the intersection of a plane (defined by P,N) with the polygon
    def calcEtaForCuttingPlane(self, P, N):
        
        nsegs = size(self.points,1)-1
        
        segfound = -1
        
        for iseg in arange(0,nsegs):
            A = self.points[:,iseg]
            B = self.points[:,iseg+1]
            segfound = iseg
            
            # get local coordinate
            denom = dot(B-A, N)
            if abs(denom) < 1e-8:
                continue
            
            s = dot(P-A, N)/denom

            # we found the intersection segment
            if s < 1:
                break
            
        # now we have to translate that into the global eta system
        eta = self.alphasOfCircleCut[segfound]*(1-s) + self.alphasOfCircleCut[segfound+1]*s
        return eta
    
    def calcPoint(self, eta):
        neta = size(self.alphasOfCircleCut)
        ifound = 0
        
        for i in arange(1,neta):
            if self.alphasOfCircleCut[i] >= eta:
                ifound = i
                break
        
        if eta > self.alphasOfCircleCut[neta-1]:
            ifound = neta-1
        elif eta < 0:
            ifound = 1        
        
        A = self.points[:,ifound-1]
        B = self.points[:,ifound]
        
        etastop = self.alphasOfCircleCut[ifound]
        etastart = self.alphasOfCircleCut[ifound-1]    
        eta_seg = (eta-etastart)/(etastop-etastart)
        
        P = A*(1-eta_seg) + B*eta_seg
        N = (B-A)
        N = N/linalg.norm(N)
        return (P, N, ifound-1)

if __name__ == '__main__':
    # some tests
    points = array ([[1., 2., 3.],
                     [1., 1.001, 1.]])
                     
    poly = PolygonWRoundedEdges(points)

    (p,n,seg) = poly.calcPoint(0.485)
    print p
    
    print poly.project(array([1.5, 0.5]))
    
    eta = 0.345345
    (p_, _, _) = poly.calcPoint(eta)
    print poly.project(p_)
    
    import matplotlib.pyplot as plt
    fig = plt.figure()
    poly.plot(fig.gca())
    plt.plot([p[0]], [p[1]], 'gx')
    
    plt.show()