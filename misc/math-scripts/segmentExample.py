import pkg_resources
pkg_resources.require("matplotlib")

from ms_segmentGeometry import *
from ms_optAlgs import *

import matplotlib as mpl
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt

# define outer point of wing segment
p1 = array([15.080271828981320681,6.3331944636655546077,-0.82588659024004418274])
p2 = array([20.620103026537961455,16.956343945599122947,-0.089370094291781887463])
p3 = array([18.836422557259382415,6.3331944636655546077,-0.95705426399850279662])
p4 = array([22.115034014736966839,16.956343945599122947,-0.14157423473675034842])

seg = SegmentGeometry(p1,p2,p3,p4);

x =  array([21.344060867260836289,16.956343945599122947,-0.10327302824713803509])

fig = plt.figure()
ax = fig.gca(projection='3d')
seg.draw(ax)
ax.plot([x[0]], [x[1]], [x[2]], 'rx')

seg.project(x)

plt.show()

