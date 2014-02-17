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

#include"testUtils.h"
#include<fstream>
#include<iomanip>

// save x-y data
void outputXY(const int & i, const double& x, const double&y, const std::string& filename)
{
    std::ofstream out;
    if (i>0) {
        out.open(filename.c_str(), std::ios::app);
    }
    else {
        out.open(filename.c_str());
    }
    out << std::setprecision(17) << std::scientific  << x << "\t" << y << std::endl;
    out.close();
} 
void outputXYVector(const int& i, const double& x, const double& y, const double& vx, const double& vy, const std::string& filename)
{
    std::ofstream out;
    if (i>0) {
        out.open(filename.c_str(), std::ios::app);
    }
    else {
        out.open(filename.c_str());
    }
    out << std::setprecision(17) << std::scientific  << x << "\t" << y << "\t" << vx << "\t" << vy << "\t" << std::endl;
    out.close();
}
