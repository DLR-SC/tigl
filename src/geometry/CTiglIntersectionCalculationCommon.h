/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2022-09-22 Anton Reiswich <Anton.Reiswich@dlr.de>
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
* @brief  Class to compute intersection algorithms.
*/


#ifndef CTIGLINTERSECTIONCALCULATIONCOMMON
#define CTIGLINTERSECTIONCALCULATIONCOMMON

#include "tigl.h"
#include "tigl_internal.h"
#include "CCPACSWings.h"
#include "CCPACSWingProfile.h"
#include "CCPACSFuselages.h"
#include "CCPACSFuselageProfile.h"
#include "CTiglIntersectionCalculation.h"

#include "GeomAPI_IntSS.hxx"
#include "ShapeAnalysis_Wire.hxx"
#include "TopTools_HSequenceOfShape.hxx"
#include "ShapeAnalysis_FreeBounds.hxx"


namespace tigl
{

class CTiglShapeCache;

class CTiglIntersectionCalculationCommon : public CTiglIntersectionCalculation
{

public:
    using CTiglIntersectionCalculation::CTiglIntersectionCalculation;

protected:
    void computeIntersection(CTiglShapeCache* cache,
                                   size_t hashOne,
                                   size_t hashTwo,
                                   TopoDS_Shape compoundOne,
                                   TopoDS_Shape compoundTwo );
};

} // end namespace tigl

#endif // CTIGLINTERSECTIONCALCULATIONCOMMON
