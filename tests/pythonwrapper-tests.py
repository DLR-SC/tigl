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

	def test_exportMeshedWingVTKByIndex(self):
		self.tigl.exportMeshedWingVTKByIndex(1, 'TestData/export/D150modelID_wing1_python.vtp', 0.01)
		
	def test_wingComponentSegmentPointGetSegmentEtaXsi(self):
		(wingUID, segmentUID, eta, xsi) = self.tigl.wingComponentSegmentPointGetSegmentEtaXsi('D150_VAMP_W1_CompSeg1', 0.0, 0.0)
		self.assertEqual(wingUID, 'D150_VAMP_W1')
		self.assertEqual(segmentUID, 'D150_VAMP_W1_Seg1')
		self.assertAlmostEqual(eta, 0.0)
		self.assertAlmostEqual(xsi, 0.0)
	
# ----------------------------------------------------------------------- #		
if __name__ == '__main__':
	unittest.main()
