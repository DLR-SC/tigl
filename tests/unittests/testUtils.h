/* 
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-17 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
/**
* @file
* @brief Helper functions for tests
*/

#ifndef TESTUTILS_H
#define TESTUTILS_H

#include<fstream>

#include <Geom_BSplineCurve.hxx>
// class TColgp_Array1OfPnt;

// save x-y data
void outputXY(const int & i, const double& x, const double&y, const std::string& filename);
void outputXYVector(const int& i, const double& x, const double& y, const double& vx, const double& vy, const std::string& filename);
void StoreResult(const std::string& filename, const Handle(Geom_BSplineCurve)& curve, const TColgp_Array1OfPnt& pt);

#endif // TESTUTILS_H

