/* 
* Copyright (C) 2016 Airbus Defence and Space
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
* @brief  Implementation of CPACS Wing Cell handling routines.
*/

#ifndef CCPACSWINGCELLPOSITIONCHORDWISE_H
#define CCPACSWINGCELLPOSITIONCHORDWISE_H

#include <string>
#include <utility>

#include <tixi.h>
#include <TopoDS_Shape.hxx>

#include "tigl_internal.h"


namespace tigl
{
// forware declarations
class CCPACSWingCell;

class CCPACSWingCellPositionChordwise
{
public:
    enum /* class */ InputType
    {
        Xsi,
        Spar,
        None
    };

    TIGL_EXPORT CCPACSWingCellPositionChordwise(CCPACSWingCell* parent);
    TIGL_EXPORT virtual ~CCPACSWingCellPositionChordwise() {} // = default;

    TIGL_EXPORT void Reset();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath);
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath) const;

    TIGL_EXPORT InputType GetInputType() const;

    TIGL_EXPORT void GetXsi(double& xsi1, double& xsi2) const;
    TIGL_EXPORT std::pair<double, double> GetXsi() const;
    TIGL_EXPORT void SetXsi(double xsi1, double xsi2);

    TIGL_EXPORT const std::string& GetSparUId() const;
    TIGL_EXPORT void SetSparUId(std::string nSparUId);

private:
    CCPACSWingCellPositionChordwise(const CCPACSWingCellPositionChordwise&); // = delete;
    void operator=(const CCPACSWingCellPositionChordwise&); // = delete;

private:
    InputType m_inputType;
    double m_xsi1;
    double m_xsi2;
    std::string m_sparUID;

    CCPACSWingCell* m_parent;
};

} // end namespace tigl
#endif // CCPACSWINGCELLPOSITIONCHORDWISE_H
