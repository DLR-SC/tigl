# Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
#
# Created: 2014-04-15 Martin Siggel <Martin.Siggel@dlr.de>
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

from __future__ import print_function

from tigl3.tigl3wrapper import Tigl3, TiglBoolean
from tixi3.tixi3wrapper import Tixi3
import sys

def python_demo(filename):
    tixi = Tixi3()
    tigl = Tigl3()
    
    # open cpacs xml with tixi
    try:
        tixi.open(filename)
    except:
        print ('Error opening cpacs file with TiXI')
        return


    # enable logging of errors and warnings into file
    tigl.logSetFileEnding('txt')
    tigl.logSetTimeInFilenameEnabled(TiglBoolean.TIGL_FALSE)
    tigl.logToFileEnabled('demolog')

    # open cpacs with tigl
    try:
        tigl.open(tixi, '')
    except:
        print ('Error opening cpacs file with TiGL')
        return
        

    # query number of wings and fuselages and their names
    nWings = tigl.getWingCount()
    nFuselages = tigl.getFuselageCount()
    for i in range(1,nWings+1):
        wingUID = tigl.wingGetUID(i)
        print ('Wing %d name: %s' % (i, wingUID))
        

    for i in range(1,nFuselages+1):
        fuselageUID = tigl.fuselageGetUID(i)
        print ('Fuselage %d name: %s' % (i, fuselageUID))


    # query point on the upper wing surface
    if nWings > 0:
        (x,y,z) = tigl.wingGetUpperPoint(1, 1, 0.5, 0.5)
        print ('Point on upper wing surface of wing 1/segment 1: (x,y,z) = (%g, %g, %g)' %(x, y, z))

    # compute intersection of wing with a x-z plane
    if nWings > 0:
        wingUID = tigl.wingGetUID(1)
        # do the intersection with a plane at p = (0,0.5,0) and normal vector n = (0,1,0)
        int_id = tigl.intersectWithPlane(wingUID, 0, 0.5, 0, 0, 1, 0)

        # get number of intersection wires
        nlines = tigl.intersectGetLineCount(int_id)

        # query points on the first line
        if nlines > 0:
            zeta = 0.
            print ('\nIntersection points of a plane with first wing:')
            while zeta < 1: 
                (x,y,z) = tigl.intersectGetPoint(int_id, 1, zeta)
                print ('zeta = %g\tp=(%g, %g, %g)' % (zeta, x, y, z))
                zeta = zeta + 0.1      
            print()

    # Export geometry to iges file
    print ('Exporting geometry to igesexport.igs')
    tigl.exportIGES('igesexport.igs')

    tigl.logToFileDisabled()
    tigl.close()
    tixi.close()
    tixi.cleanup()

def print_usage():
    print ('python_demo <cpacs file>')

# main program
def main(argv):
    if len(argv) != 2:
        print_usage()
        return
        
    filename = argv[1]
    python_demo(filename)
    
if __name__ == "__main__":
   main(sys.argv)
