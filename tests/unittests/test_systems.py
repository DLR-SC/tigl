#############################################################################
# Copyright (C) 2007-2025 German Aerospace Center (DLR/SC)
#
# Created: 2025-30-01 Marko Alder <marko.alder@dlr.de>
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

from tigl3.tigl3wrapper import *
from tixi3.tixi3wrapper import *
from tigl3 import configuration


class Systems(unittest.TestCase):

    def setUp(self):
        self.tixi = Tixi3()
        self.tigl = Tigl3()
        self.assertIsNone(self.tixi.open("TestData/simpletest-systems.cpacs.xml"))
        self.assertIsNone(self.tigl.open(self.tixi, ""))

        mgr = configuration.CCPACSConfigurationManager.get_instance()
        self.config = mgr.get_configuration(self.tigl._handle.value)
        uid_mgr = self.config.get_uidmanager()

        self.genSys = uid_mgr.get_geometric_component("genSys_1")

    def tearDown(self):
        self.tigl.close()
        self.tixi.close()

    def test_genericSystem(self):

        # Access to CPACSGenericSystem
        self.assertEqual(self.genSys.get_name(), "Generic System 1")
        self.assertEqual(self.genSys.get_uid(), "genSys_1")

        # Access to CCPACSGenericSystem
        self.assertEqual(self.genSys.get_defaulted_uid(), "genSys_1")
        self.assertIsInstance(
            self.genSys.get_components(), configuration.CCPACSComponents
        )


if __name__ == "__main__":
    unittest.main()
