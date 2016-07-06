/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-10 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
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
* @brief  Implementation of a CPACS guide curve profile
*/

#ifndef CCPACSGUIDECURVE_H
#define CCPACSGUIDECURVE_H

#include "generated/CPACSGuideCurve.h"
#include "tigl_internal.h"
#include "tixi.h"
#include "CSharedPtr.h"

#include <gp_Pnt.hxx>

#include <vector>
#include <string>

namespace tigl
{

class CTiglPoint;
typedef class CSharedPtr<CTiglPoint> PCTiglPoint;

class CCPACSGuideCurve : public generated::CPACSGuideCurve
{
public:
    // Constructor
    TIGL_EXPORT CCPACSGuideCurve();

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSGuideCurve(void);

    // Read CPACS guide curve file
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);

    // Check if fromRelativeCircumference is set
    TIGL_EXPORT bool GetFromRelativeCircumferenceIsSet(void) const;

    // Returns the relative circumference of the starting profile
    TIGL_EXPORT double GetFromRelativeCircumference(void) const;

    // Returns the relative circumference of the end profile
    TIGL_EXPORT double GetToRelativeCircumference(void) const;

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    bool fromRelativeCircumferenceIsSet;  /**< Determine if fromRelativeCircumference is set */
};

} // end namespace tigl

#endif // CCPACSGUIDECURVE_H

