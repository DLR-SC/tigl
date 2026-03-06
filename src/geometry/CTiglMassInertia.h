/*
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-03-05 Marko Alder <marko.alder@dlr.de>
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
* @brief  Implementation of a TIGL mass inertia vector.
*/

#pragma once

#include <boost/optional.hpp>

namespace tigl
{
struct CTiglMassInertia {
    double Jxx = 0.0;
    double Jyy = 0.0;
    double Jzz = 0.0;

    boost::optional<double> Jxy = boost::none;
    boost::optional<double> Jxz = boost::none;
    boost::optional<double> Jyz = boost::none;
};

} // namespace tigl