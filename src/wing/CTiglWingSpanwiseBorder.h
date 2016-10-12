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
* @brief  Base class for CCPACSWingCellPositionSpanwise and CCPACSControlSurfaceSkinCutOutBorder
*/

#ifndef CTIGLWINGSPANWISEBORDER_H
#define CTIGLWINGSPANWISEBORDER_H

#include <string>
#include <tixi.h>
#include "tigl_internal.h"


namespace tigl 
{

class CTiglWingSpanwiseBorder
{
public:
    enum /* class */ InputType
    {
        Eta,
        Rib,
        None
    };

    TIGL_EXPORT CTiglWingSpanwiseBorder();
    TIGL_EXPORT virtual ~CTiglWingSpanwiseBorder() {}; // = default;

    TIGL_EXPORT void Reset();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle,  const std::string& xpath);
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle,  const std::string& xpath) const;

    TIGL_EXPORT InputType GetInputType() const;

    TIGL_EXPORT void GetEta(double& eta1, double& eta2) const;
    TIGL_EXPORT std::pair<double, double> GetEta() const;
    TIGL_EXPORT void SetEta(double eta1, double eta2);

    TIGL_EXPORT void GetRib(std::string& ribUid, int& ribNumber) const;
    TIGL_EXPORT std::pair<std::string, int> GetRib() const;
    TIGL_EXPORT void SetRib(const std::string& ribUid, int ribNumber);

protected:
    virtual void invalidateParent() = 0;

    std::string m_eta1CPACSName;
    std::string m_eta2CPACSName;

private:
    CTiglWingSpanwiseBorder(const CTiglWingSpanwiseBorder&); // = delete;
    void operator=(const CTiglWingSpanwiseBorder&); // = delete;

     InputType m_inputType;

    double m_eta1;
    double m_eta2;
    int m_ribNumber;
    std::string m_ribDefinitionUID;
};

} // end namespace tigl


#endif // CTIGLWINGSPANWISEBORDER_H
