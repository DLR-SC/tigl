#! /usr/bin/env python
#############################################################################
# Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
#
# Created: 2013-03-13 Martin Siggel <martin.siggel@dlr.de>
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
#############################################################################

import unittest
from tiglwrapper import *
from tixiwrapper import *

class TestSimpleCpacs(unittest.TestCase):

	def setUp(self):
		self.tigl = Tigl()
		self.tixi = Tixi()
		self.tixi.open('TestData/simpletest.cpacs.xml')
		self.tigl.open(self.tixi, '')
		
	def tearDown(self):
		self.tigl.close()
		self.tixi.close()

	def test_tiglGetVersion(self):
		print 'TIGL-Version: ', self.tigl.version
		
	def test_objectCount(self):
		self.assertEqual(self.tigl.getWingCount(),1)
		self.assertEqual(self.tigl.getFuselageCount(),0)
		self.assertEqual(self.tigl.wingGetComponentSegmentCount(1),1)
		
		
# ----------------------------------------------------------------------- #
# The following tests should only check, if the python api is correct.
# Correctness of TIGL functions is not tested here (this is done in the c-tests)
class TestTiglApi(unittest.TestCase):
	def setUp(self):
		self.tigl = Tigl()
		self.tixi = Tixi()
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
		segmentIndex = self.tigl.wingGetSegmentIndex(1,'D150_VAMP_W1_Seg1')
		self.assertEqual(segmentIndex, 1)
		
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
	
	def test_wingGetSurfaceArea(self):
		area = self.tigl.wingGetSurfaceArea(1);
		self.assertGreater(area, 125.)
		self.assertLess(area, 135.)
		
	def test_wingGetReferenceArea(self):
		area = self.tigl.wingGetReferenceArea(1);
		self.assertGreater(area, 60.)
		self.assertLess(area, 70.)

	def test_exportMeshedWingVTKByIndex(self):
		self.tigl.exportMeshedWingVTKByIndex(1, 'TestData/export/D150modelID_wing1_python.vtp', 0.01)
		
	
# ----------------------------------------------------------------------- #		
if __name__ == '__main__':
	unittest.main()
