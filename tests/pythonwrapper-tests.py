#! /usr/bin/env python
# -*- coding: cp1252 -*-
#############################################################################
# Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
#
# Created: 2013-03-13 Martin Siggel <martin.siggel@dlr.de>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#############################################################################

import unittest
import subprocess
from tiglwrapper import *
from tixiwrapper import *

skipSlowFunctions = True # Saves you 14 minutes(if True), but leaves out 8 functions

class TestSimpleCpacs(unittest.TestCase):

    def setUp(self):
        self.tixi = Tixi()
        self.tigl = Tigl()
        self.tixi.open('TestData/simpletest.cpacs.xml')
        self.tigl.open(self.tixi, '')
        
    def tearDown(self):
        self.tigl.close()
        self.tixi.close()

    def test_tiglGetVersion(self):
        print('TIGL-Version: ' + self.tigl.version)
        
    def test_objectCount(self):
        self.assertEqual(self.tigl.getWingCount(),1)
        self.assertEqual(self.tigl.getFuselageCount(),1)
        self.assertEqual(self.tigl.wingGetComponentSegmentCount(1),1)

    ########## Exports are faster in this class
    def test_exportIGES(self):
        filenamePtr = "TestData/export/export0.igs"
        self.tigl.exportIGES(filenamePtr)
    
    def test_exportFusedWingFuselageIGES(self):
        if not skipSlowFunctions:
            filenamePtr = "TestData/export/export.igs"
            self.tigl.exportFusedWingFuselageIGES(filenamePtr)
        
    def test_exportSTEP(self):
        filenamePtr = 'TestData/export/export.step'
        self.tigl.exportSTEP(filenamePtr)
            
    def test_exportMeshedWingSTL(self):
        if not skipSlowFunctions:
            wingIndex = 1
            filenamePtr = "TestData/export/export.stl"
            deflection = 0.01
            self.tigl.exportMeshedWingSTL(wingIndex, filenamePtr, deflection)
                    
    def test_exportMeshedGeometrySTL(self):
        if not skipSlowFunctions:
            filenamePtr = "TestData/export/export3.stl"
            deflection = 0.01
            self.tigl.exportMeshedGeometrySTL(filenamePtr, deflection)
            
    def test_exportMeshedWingVTKByUID(self):
        wingUID = "Wing"
        filenamePtr ="TestData/export/export.vtk"
        deflection = 0.01
        self.tigl.exportMeshedWingVTKByUID(wingUID, filenamePtr, deflection)
            
    def test_exportMeshedGeometryVTK(self):
        if not skipSlowFunctions:
            filenamePtr = "TestData/export/export4.vtk"
            deflection = 0.01
            self.tigl.exportMeshedGeometryVTK(filenamePtr, deflection)
            
    def test_exportMeshedWingVTKSimpleByUID(self):
        wingUID = "Wing"
        filenamePtr = "TestData/export/export5.vtk"
        deflection = 0.01
        self.tigl.exportMeshedWingVTKSimpleByUID(wingUID, filenamePtr, deflection)
            
    def test_exportMeshedGeometryVTKSimple(self):
        if not skipSlowFunctions:
            filenamePtr = "TestData/export/export7.vtk"
            deflection = 0.01
            self.tigl.exportMeshedGeometryVTKSimple(filenamePtr, deflection)
            
    def test_getMaterialUID(self):
        compSegmentUID = "WING_CS1"
        eta = 0.25
        xsi = 0.9
        nmaterials = self.tigl.wingComponentSegmentGetMaterialCount(compSegmentUID, TiglStructureType.UPPER_SHELL, eta, xsi )
        self.assertEqual(nmaterials, 1)

        material = self.tigl.wingComponentSegmentGetMaterialUID(compSegmentUID, TiglStructureType.UPPER_SHELL, eta, xsi, 1)
        self.assertEqual(material, 'MyCellMat')
######      
        
# Tests for tigl logging mechanism
class TestTiglLogging(unittest.TestCase):
    def get_logs(self, loglevel):
        process=subprocess.Popen(['python', 'test_logging.py', "%s" % loglevel], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err=process.communicate()
        status=process.returncode
        # seperate output lines
        out=out.decode("UTF-8").rstrip('\n').split('\n')
        err=err.decode("UTF-8").rstrip('\n').split('\n')
        return (status, out, err)

    def test_test_script(self):
        for lev in range(0,4):
            self.assertEqual(self.get_logs(lev)[0], 0)

    def test_info(self):
        status, out, err = self.get_logs(3)
        self.assertTrue(len(out), 1)
        self.assertTrue(len(err), 4)
        self.assertTrue(out[0].startswith('INF'))
        self.assertTrue(out[0].endswith('No far-field defined.'))
        self.assertTrue(err[0].startswith('WRN'))
        self.assertTrue(err[0].endswith('CPACS dataset version is higher than TIGL library version!'))
        self.assertTrue(err[1].startswith('WRN'))
        self.assertTrue(err[1].endswith("The points in profile PointListExampleAirfoil don't seem to be ordered in a mathematical positive sense."))
        self.assertTrue(err[2].startswith('WRN'))
        self.assertTrue(err[2].endswith('Please correct the wing profile!'))
        self.assertTrue(err[3].startswith('ERR'))
        self.assertTrue(err[3].endswith('Error: Invalid uid in tiglWingComponentSegmentPointGetSegmentEtaXsi'))

    def test_warning(self):
        status, out, err = self.get_logs(2)
        self.assertTrue(len(out), 0)
        self.assertTrue(len(err), 4)
        self.assertTrue(err[0].startswith('WRN'))
        self.assertTrue(err[0].endswith('CPACS dataset version is higher than TIGL library version!'))
        self.assertTrue(err[1].startswith('WRN'))
        self.assertTrue(err[1].endswith("The points in profile PointListExampleAirfoil don't seem to be ordered in a mathematical positive sense."))
        self.assertTrue(err[2].startswith('WRN'))
        self.assertTrue(err[2].endswith('Please correct the wing profile!'))
        self.assertTrue(err[3].startswith('ERR'))
        self.assertTrue(err[3].endswith('Error: Invalid uid in tiglWingComponentSegmentPointGetSegmentEtaXsi'))

    def test_error(self):
        status, out, err = self.get_logs(1)
        self.assertTrue(len(out), 0)
        self.assertTrue(len(err), 1)
        self.assertTrue(err[0].startswith('ERR'))
        self.assertTrue(err[0].endswith('Error: Invalid uid in tiglWingComponentSegmentPointGetSegmentEtaXsi'))

    def test_silent(self):
        status, out, err = self.get_logs(0)
        self.assertTrue(len(out), 0)
        self.assertTrue(len(err), 0)

    def test_file_logging(self):
        status, out, err = self.get_logs(0)
        f=open("test_logging.log", 'r')
        lines=f.readlines()
        f.close()
        lout=[line.rstrip('\n') for line in lines]
        self.assertTrue(len(lout), 6)
        self.assertTrue(lout[0].startswith('TiGL log file created at'))
        self.assertTrue(lout[1].startswith('WRN'))
        self.assertTrue(lout[1].endswith('CPACS dataset version is higher than TIGL library version!'))
        self.assertTrue(lout[2].startswith('WRN'))
        self.assertTrue(lout[2].endswith("The points in profile PointListExampleAirfoil don't seem to be ordered in a mathematical positive sense."))
        self.assertTrue(lout[3].startswith('WRN'))
        self.assertTrue(lout[3].endswith('Please correct the wing profile!'))
        self.assertTrue(lout[4].startswith('INF'))
        self.assertTrue(lout[4].endswith('No far-field defined.'))
        self.assertTrue(lout[5].startswith('ERR'))
        self.assertTrue(lout[5].endswith('Error: Invalid uid in tiglWingComponentSegmentPointGetSegmentEtaXsi'))

# ----------------------------------------------------------------------- #
# The following tests should only check, if the python api is correct.
# Correctness of TIGL functions is not tested here (this is done in the c-tests)
class TestTiglApi(unittest.TestCase):
    def setUp(self):
        self.tigl = Tigl()
        self.tixi = Tixi()
        self.tigl.logSetVerbosity(TiglLogLevel.TILOG_SILENT)
        self.tixi.open('TestData/CPACS_21_D150.xml')
        self.tigl.open(self.tixi, 'D150_VAMP')
    
    def tearDown(self):
        self.tigl.close()
        self.tixi.close()
        
    def test_getWingCount(self):
        self.assertEqual(self.tigl.getWingCount(),3)
        
    def test_getFuselageCount(self):
        self.assertEqual(self.tigl.getFuselageCount(),1)
    
    def test_wingGetSegmentCount(self):
        self.assertEqual(self.tigl.wingGetSegmentCount(1),3)
        self.assertEqual(self.tigl.wingGetSegmentCount(2),1)
        self.assertEqual(self.tigl.wingGetSegmentCount(3),1)
        
        # check illegal wing index error handling
        error = TiglReturnCode.TIGL_SUCCESS
        try:
            self.tigl.wingGetSegmentCount(-1);
        except TiglException as e:
            error = e.code  
        self.assertEqual(error, TiglReturnCode.TIGL_INDEX_ERROR) 
    
    def test_wingGetComponentSegmentCount(self):
        self.assertEqual(self.tigl.wingGetComponentSegmentCount(1),1)
        self.assertEqual(self.tigl.wingGetComponentSegmentCount(2),1)
        self.assertEqual(self.tigl.wingGetComponentSegmentCount(3),1)
    
    def test_wingGetComponentSegmentUID(self):
        wingCompUID = self.tigl.wingGetComponentSegmentUID(1,1)
        self.assertEqual(wingCompUID, 'D150_VAMP_W1_CompSeg1')
        wingCompUID = self.tigl.wingGetComponentSegmentUID(2,1)
        self.assertEqual(wingCompUID, 'D150_VAMP_HL1_CompSeg1')
        wingCompUID = self.tigl.wingGetComponentSegmentUID(3,1)
        self.assertEqual(wingCompUID, 'D150_VAMP_SL1_CompSeg1')
        
    def test_wingGetComponentSegmentIndex(self):
        compSegIndex = self.tigl.wingGetComponentSegmentIndex(1,'D150_VAMP_W1_CompSeg1')
        self.assertEqual(compSegIndex,1)

    def test_wingGetUpperPoint(self):
        (x, y, z) = self.tigl.wingGetUpperPoint(1,1,0.5,0.5);
        
    def test_wingGetLowerPoint(self):
        (x, y, z) = self.tigl.wingGetLowerPoint(1,1,0.5,0.5);
        
    def test_wingGetInnerConnectedSegmentCount(self):
        segmentCount = self.tigl.wingGetInnerConnectedSegmentCount(1,1)
        self.assertEqual(segmentCount,0)
        
    def test_wingGetOuterConnectedSegmentCount(self):
        segmentCount = self.tigl.wingGetOuterConnectedSegmentCount(1,1)
        self.assertEqual(segmentCount,1)

    def test_wingGetInnerConnectedSegmentIndex(self):
        try:
            segIndex = self.tigl.wingGetInnerConnectedSegmentIndex(1,1,0)
            self.assertEqual(True, False)
        except TiglException as e:
            self.assertEqual(e.code, TiglReturnCode.TIGL_INDEX_ERROR)
    
    def test_wingGetOuterConnectedSegmentIndex(self):
        segIndex = self.tigl.wingGetOuterConnectedSegmentIndex(1,1,1)
        self.assertEqual(segIndex,2)

    def test_wingGetInnerSectionAndElementIndex(self):
        (secindex, elementindex) = self.tigl.wingGetInnerSectionAndElementIndex(1,1)
        self.assertEqual(secindex, 1)
        self.assertEqual(elementindex,1)
        
    def test_wingGetOuterSectionAndElementIndex(self):
        (secindex, elementindex) = self.tigl.wingGetOuterSectionAndElementIndex(1,1)
        self.assertEqual(secindex, 2)
        self.assertEqual(elementindex,1)
    
    def test_wingGetInnerSectionAndElementUID(self):
        (secUID, elementUID) = self.tigl.wingGetInnerSectionAndElementUID(1,1)
        self.assertEqual(secUID, 'D150_VAMP_W1_Sec1')
        self.assertEqual(elementUID, 'D150_VAMP_W1_Sec1_Elem1') 

    def test_wingGetOuterSectionAndElementUID(self):
        (secUID, elementUID) = self.tigl.wingGetOuterSectionAndElementUID(1,1)
        self.assertEqual(secUID, 'D150_VAMP_W1_Sec2')
        self.assertEqual(elementUID, 'D150_VAMP_W1_Sec2_Elem1')     
    
    def test_wingGetProfileName(self):
        profileName = self.tigl.wingGetProfileName(1,1,1)
        self.assertEqual(profileName, 'NameD150_VAMP_W_SupCritProf1')
        
    def test_wingGetUID(self):
        wingUID = self.tigl.wingGetUID(1)
        self.assertEqual(wingUID, 'D150_VAMP_W1')
    
    def test_wingGetIndex(self):
        wingIndex = self.tigl.wingGetIndex('D150_VAMP_W1')
        self.assertEqual(wingIndex, 1)  
        
    def test_wingGetSegmentUID(self):
        segmentUID = self.tigl.wingGetSegmentUID(1,1)
        self.assertEqual(segmentUID, 'D150_VAMP_W1_Seg1')
        
    def test_wingGetSegmentIndex(self):
        segmentIndex, wingIndex = self.tigl.wingGetSegmentIndex('D150_VAMP_W1_Seg1')
        self.assertEqual(segmentIndex, 1)
        self.assertEqual(wingIndex, 1)
        
    def test_wingGetSectionUID(self):
        sectionUID = self.tigl.wingGetSectionUID(1,1)
        self.assertEqual(sectionUID, 'D150_VAMP_W1_Sec1')
        
    def test_wingGetSymmetry(self):
        symm = self.tigl.wingGetSymmetry(1)
        self.assertEqual(symm, TiglSymmetryAxis.TIGL_X_Z_PLANE)
        
    def test_wingComponentSegmentFindSegment(self):
        (x, y, z) = self.tigl.wingGetUpperPoint(1,1,0.5,0.5);
        (segUID, wingUID) = self.tigl.wingComponentSegmentFindSegment('D150_VAMP_W1_CompSeg1',x,y,z)
        self.assertEqual(segUID, 'D150_VAMP_W1_Seg1')
        self.assertEqual(wingUID, 'D150_VAMP_W1')

    def test_wingComponentSegmentPointGetSegmentEtaXsi(self):
        (wingUID, segmentUID, eta, xsi) = self.tigl.wingComponentSegmentPointGetSegmentEtaXsi('D150_VAMP_W1_CompSeg1', 0.0, 0.0)
        self.assertEqual(wingUID, 'D150_VAMP_W1')
        self.assertEqual(segmentUID, 'D150_VAMP_W1_Seg1')
        self.assertAlmostEqual(eta, 0.0)
        self.assertAlmostEqual(xsi, 0.0)

    def test_wingSegmentPointGetComponentSegmentEtaXsi(self):
        (eta, xsi) = self.tigl.wingSegmentPointGetComponentSegmentEtaXsi('D150_VAMP_W1_Seg1', 'D150_VAMP_W1_CompSeg1', 0.0, 0.0)
        self.assertAlmostEqual(eta, 0.0)
        self.assertAlmostEqual(xsi, 0.0)
        
    def test_getFuselageCount(self):
        fc = self.tigl.getFuselageCount()
        self.assertEqual(fc,1)
    
    def test_fuselageGetSegmentCount(self):
        sc = self.tigl.fuselageGetSegmentCount(1)
        self.assertEqual(sc,59)
        
    def test_fuselageGetPoint(self):
        for iseg in range(0,59):
            (x,y,z) = self.tigl.fuselageGetPoint(1,iseg+1,0.5,0.5)
        #check for illegal index
        try:    
            self.tigl.fuselageGetPoint(1,60,0.5,0.5)
            self.assertEqual(False, True)
        except TiglException as e:
            self.assertEqual(e.code, TiglReturnCode.TIGL_INDEX_ERROR)
    
    def test_fuselageGetPointAngle(self):
        wingIDX = 1
        segIDX = 40
        (x,y,z) = self.tigl.fuselageGetPointAngle(wingIDX, segIDX, 0.5, 90.0)
        
    def test_fuselageGetPointAngleTranslated(self):
        fuselIDX = 1
        segIDX = 40
        ytrans = 0.1;
        ztrans = -0.1;
        (x,y,z) = self.tigl.fuselageGetPointAngleTranslated(fuselIDX, segIDX, 0.5, 90.0, ytrans, ztrans)
        
    def test_fuselageGetPointOnXPlane(self):
        fuselIDX = 1
        segIDX = 40
        zpos = 0.7
        (x,y,z) = self.tigl.fuselageGetPointOnXPlane(fuselIDX, segIDX,  0.5, zpos, 1)
        self.assertAlmostEqual(zpos, z)
    
    def test_fuselageGetNumPointsOnXPlane(self):
        fuselIDX = 1
        segIDX = 40
        zpos = 0.7
        num = self.tigl.fuselageGetNumPointsOnXPlane(fuselIDX, segIDX,  0.5, zpos)
        self.assertGreater(num, 0)

    def test_fuselageGetPointOnYPlane(self):
        fuselIDY = 1
        segIDY = 40
        zpos = 0.7
        (x,y,z) = self.tigl.fuselageGetPointOnYPlane(fuselIDY, segIDY,  0.5, zpos, 1)
        self.assertAlmostEqual(zpos, z)
    
    def test_fuselageGetNumPointsOnYPlane(self):
        fuselIDY = 1
        segIDY = 40
        zpos = 0.7
        num = self.tigl.fuselageGetNumPointsOnYPlane(fuselIDY, segIDY,  0.5, zpos)
        self.assertGreater(num, 0)
        
############################ ANFANG
    def test_fuselageGetCircumference(self):
        fuselageIndex = 1
        segmentIndex = 1
        eta = 0.5
        ret = self.tigl.fuselageGetCircumference(fuselageIndex, segmentIndex, eta)
        self.assertEqual(isinstance(ret, float),True)
            
    def test_fuselageGetStartConnectedSegmentCount(self):
        fuselageIndex = 1
        segmentIndex = 1 
        ret = self.tigl.fuselageGetStartConnectedSegmentCount(fuselageIndex, segmentIndex)
        self.assertEqual(ret,0)
            
    def test_fuselageGetEndConnectedSegmentCount(self):
        fuselageIndex = 1
        segmentIndex = 1
        ret = self.tigl.fuselageGetEndConnectedSegmentCount(fuselageIndex, segmentIndex)
        self.assertEqual(ret,1)
            
    def test_fuselageGetStartConnectedSegmentIndex(self):
        fuselageIndex = 1
        segmentIndex = 2
        n = 1
        ret = self.tigl.fuselageGetStartConnectedSegmentIndex(fuselageIndex, segmentIndex, n)
        self.assertEqual(ret,1)
            
    def test_fuselageGetEndConnectedSegmentIndex(self):
        fuselageIndex = 1
        segmentIndex = 1
        n = 1
        ret = self.tigl.fuselageGetEndConnectedSegmentIndex(fuselageIndex, segmentIndex, n)
        self.assertEqual(ret,2)
            
    def test_fuselageGetStartSectionAndElementUID(self):
        fuselageIndex = 1
        segmentIndex = 1
        n = 1
        ret = self.tigl.fuselageGetStartSectionAndElementUID(fuselageIndex, segmentIndex)
        self.assertEqual(ret,('D150_VAMP_FL1_Sec1', 'D150_VAMP_FL1_Sec1_Elem1'))
            
    def test_fuselageGetEndSectionAndElementUID(self):
        fuselageIndex = 1
        segmentIndex = 1
        ret = self.tigl.fuselageGetEndSectionAndElementUID(fuselageIndex, segmentIndex)
        self.assertEqual(ret,('D150_VAMP_FL1_Sec2', 'D150_VAMP_FL1_Sec2_Elem1'))
            
    def test_fuselageGetStartSectionAndElementIndex(self):
        fuselageIndex = 1
        segmentIndex = 2
        ret = self.tigl.fuselageGetStartSectionAndElementIndex(fuselageIndex, segmentIndex)
        self.assertEqual(ret,(2,1))
            
    def test_fuselageGetEndSectionAndElementIndex(self):
        fuselageIndex = 1
        segmentIndex = 1
        ret = self.tigl.fuselageGetEndSectionAndElementIndex(fuselageIndex, segmentIndex)
        self.assertEqual(ret,(2,1))
            
    def test_fuselageGetProfileName(self):
        fuselageIndex = 1
        sectionIndex = 1
        elementIndex = 1 
        ret = self.tigl.fuselageGetProfileName(fuselageIndex, sectionIndex, elementIndex)
        self.assertEqual(ret,"Circle")
            
    def test_fuselageGetUID(self):
        fuselageIndex = 1
        ret = self.tigl.fuselageGetUID(fuselageIndex)
        self.assertEqual(ret,'D150_VAMP_FL1')
            
    def test_fuselageGetSegmentUID(self):
        fuselageIndex = 1
        segmentIndex = 1
        ret = self.tigl.fuselageGetSegmentUID(fuselageIndex, segmentIndex)
        self.assertEqual(ret,'D150_VAMP_FL1_Seg1')
            
    def test_fuselageGetSectionUID(self):
        fuselageIndex = 1
        sectionIndex = 1
        ret = self.tigl.fuselageGetSectionUID(fuselageIndex, sectionIndex)
        self.assertEqual(isinstance(ret,str),True)
            
    def test_fuselageGetSymmetry(self):
        fuselageIndex = 1
        ret = self.tigl.fuselageGetSymmetry(fuselageIndex)
        self.assertEqual(ret,0)
            
## Raises TiglException: TIGL_ERROR (1) (both give the same Error)          
##  def test_componentIntersectionPoint(self):
##      print("IntersectionPoint")
##      componentUidOne = self.tigl.fuselageGetUID(1)
##      componentUidTwo = self.tigl.wingGetUID(1)
##      lineID = 1
##      eta = 0.5
##      ret = self.tigl.componentIntersectionPoint(componentUidOne, componentUidTwo, lineID, eta)
##      self.assertEqual((len(ret)==3),True)
##          
##  def test_componentIntersectionLineCount(self):
##      print("IntersectionLine")
##      componentUidOne = self.tigl.fuselageGetUID(1)
##      componentUidTwo = self.tigl.wingGetUID(1)
##      ret = self.tigl.componentIntersectionLineCount(componentUidOne, componentUidTwo)
        
            
    def test_wingGetVolume(self):
        wingIndex = 1 
        ret = self.tigl.wingGetVolume(wingIndex)
        self.assertEqual(isinstance(ret,float),True)
            
    def test_wingGetSegmentVolume(self):
        wingIndex = 1
        segmentIndex = 1
        ret = self.tigl.wingGetSegmentVolume(wingIndex, segmentIndex)
        self.assertEqual(isinstance(ret,float),True)
            
    def test_fuselageGetSegmentVolume(self):
        fuselageIndex = 1
        segmentIndex = 1
        ret = self.tigl.fuselageGetSegmentVolume(fuselageIndex, segmentIndex)
        self.assertEqual(isinstance(ret,float),True)
            
    def test_wingGetSurfaceArea(self):
        wingIndex = 1
        ret = self.tigl.wingGetSurfaceArea(wingIndex)
        self.assertEqual(isinstance(ret,float),True)
            
    def test_wingGetSegmentSurfaceArea(self):
        wingIndex = 1
        segmentIndex = 1
        ret = self.tigl.wingGetSegmentSurfaceArea(wingIndex, segmentIndex)
        self.assertEqual(isinstance(ret,float),True)            

    def test_fuselageGetSegmentSurfaceArea(self):
        fuselageIndex = 1
        segmentIndex = 1
        ret = self.tigl.fuselageGetSegmentSurfaceArea(fuselageIndex, segmentIndex)
        self.assertEqual(isinstance(ret,float),True)
            
    def test_componentGetHashCode(self):
        componentUID = self.tigl.wingGetUID(1)
        ret = self.tigl.componentGetHashCode(componentUID)
        self.assertEqual(isinstance(ret,int),True)
                    
    def test_configurationGetLength(self):
        ret = self.tigl.configurationGetLength()
        self.assertEqual(isinstance(ret,float),True)
            
    def test_wingGetSpan(self):
        wingUID = self.tigl.wingGetUID(1)
        ret = self.tigl.wingGetSpan(wingUID)
        self.assertEqual(isinstance(ret,float),True)
        
    def test_wingGetSurfaceArea(self):
        area = self.tigl.wingGetSurfaceArea(1);
        self.assertGreater(area, 125.)
        self.assertLess(area, 135.)
        
    def test_wingGetReferenceArea(self):
        area = self.tigl.wingGetReferenceArea(1, TiglSymmetryAxis.TIGL_X_Y_PLANE);
        self.assertGreater(area, 60.)
        self.assertLess(area, 70.)

    def test_exportMeshedWingVTKByIndex(self):
        self.tigl.exportMeshedWingVTKByIndex(1, 'TestData/export/D150modelID_wing1_python.vtp', 0.01)

###### SLOW Function, basically fuselage based due to complex fuselage geometry
    def test_MergedTests(self):
        if skipSlowFunctions:
            return
        print( "Slow Functions (14min)")
        
        #exportMeshedFuselageSTL
        fuselageIndex = 1
        filenamePtr = "TestData/export/export2.stl"
        deflection = 0.01
        self.tigl.exportMeshedFuselageSTL(fuselageIndex, filenamePtr, deflection)

        # Fuselage VTK by Index
        fuselageIndex = 1
        filenamePtr = "TestData/export/export2.vtk"
        deflection = 0.01
        self.tigl.exportMeshedFuselageVTKByIndex(fuselageIndex, filenamePtr, deflection)

        # Fuselage VTk by UID
        fuselageUID = "D150_VAMP_FL1"
        filenamePtr = "TestData/export/export3.vtk"
        deflection = 0.01
        self.tigl.exportMeshedFuselageVTKByUID(fuselageUID, filenamePtr, deflection)

        # Fuselage Simple-VTK by UID    
        fuselageUID = "D150_VAMP_FL1"
        filenamePtr ="TestData/export/export6.vtk"
        deflection = 0.01
        self.tigl.exportMeshedFuselageVTKSimpleByUID(fuselageUID, filenamePtr, deflection)

        # Distance to ground
        fuselageUID = self.tigl.fuselageGetUID(1)
        axisPntX = 0
        axisPntY = 0
        axisPntZ = 10
        axisDirX = 0
        axisDirY = 0
        axisDirZ = 1
        angle = 0
        ret = self.tigl.fuselageGetMinumumDistanceToGround(fuselageUID, axisPntX, axisPntY, axisPntZ, axisDirX, axisDirY, axisDirZ, angle)

        # Volume 
        ret = self.tigl.fuselageGetVolume(fuselageIndex)
        self.assertEqual(isinstance(ret,float),True)

        # Wetted Area
        print("50%")
        wingUID = self.tigl.wingGetUID(1)
        ret = self.tigl.wingGetWettedArea(wingUID)
        self.assertEqual(isinstance(ret,float),True)

        # Fuselage Surface Area
        fuselageIndex = 1
        ret = self.tigl.fuselageGetSurfaceArea(fuselageIndex)
        self.assertEqual(isinstance(ret,float),True)
        print("100%")
# ----------------------------------------------------------------------- #     
if __name__ == '__main__':
    unittest.main()
