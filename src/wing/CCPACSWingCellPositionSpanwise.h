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

#ifndef CCPACSWINGCELLPOSITIONSPANWISE_H
#define CCPACSWINGCELLPOSITIONSPANWISE_H

#include "generated/CPACSCellPositioningSpanwise.h"

namespace tigl
{

// forward declarations
class CCPACSWingCell;

class CCPACSWingCellPositionSpanwise : public generated::CPACSCellPositioningSpanwise
{
public:
    enum InputType
    {
        Eta,
        Rib
    };

    TIGL_EXPORT CCPACSWingCellPositionSpanwise(CCPACSWingCell* parent);

    TIGL_EXPORT InputType GetInputType() const;

    TIGL_EXPORT void GetEta(double& eta1, double& eta2) const;
    TIGL_EXPORT std::pair<double, double> GetEta() const;
    TIGL_EXPORT void SetEta1(double eta1);
    TIGL_EXPORT void SetEta2(double eta2);
    TIGL_EXPORT void SetEta(double eta1, double eta2);
    TIGL_EXPORT void SetEta(double eta1, const std::string& eta1RefUid, double eta2, const std::string& eta2RefUid);

    TIGL_EXPORT void GetRib(std::string& ribUid, int& ribNumber) const;
    TIGL_EXPORT std::pair<std::string, int> GetRib() const;
    TIGL_EXPORT void SetRibDefinitionUID(const std::string& ribUid);
    TIGL_EXPORT void SetRibNumber(int ribNumber);
    TIGL_EXPORT void SetRib(const std::string& ribUid, int ribNumber);

protected:
    void InvalidateParent();

    friend class CCPACSEtaIsoLine;
};

} // end namespace tigl


#endif // CCPACSWINGCELLPOSITIONSPANWISE_H
