#
# Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
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
# @file ms_segmentGeometry.py
# @brief Implementation of the segement geometry
#
# @todo Further implementation required
#

from numpy import *

def ms_calcSegmentPoint(x1,x2,x3,x4, alpha, beta):

	a = -x1+x2;
	b = -x1+x3;
	c = x1-x2-x3+x4;
	d = x1;

	return outer(a,alpha) + outer(b,beta) + outer(c,alpha*beta) + outer(d,ones(size(alpha)));

