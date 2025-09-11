/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-27 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CCPACSTRANSFORMATION_H
#define CCPACSTRANSFORMATION_H

#include "generated/CPACSTransformation.h"

#include "CTiglTransformation.h"
#include "ECPACSTranslationType.h"
#include "Cache.h"

namespace tigl
{


class CCPACSTransformation : public generated::CPACSTransformation
{
public:
    TIGL_EXPORT CCPACSTransformation(CCPACSEnginePosition* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSEnginePylon* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSFuselage* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSDuct* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSDuctAssembly* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSFuselageSectionElement* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSFuselageSection* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSExternalObject* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSGenericSystem* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSVessel* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSFuelTank* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSGenericGeometryComponent* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSLandingGearBase* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSNacelleSection* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSRotor* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSRotorHinge* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSWing* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSWingSectionElement* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformation(CCPACSWingSection* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT CCPACSTransformation(CTiglUIDManager* uidMgr);
    
    TIGL_EXPORT void reset();

    TIGL_EXPORT CCPACSTransformation &operator = (const CCPACSTransformation& trafo);
    
    TIGL_EXPORT void setTranslation(const CTiglPoint& translation);
    TIGL_EXPORT void setTranslation(const CTiglPoint& translation, ECPACSTranslationType);
    TIGL_EXPORT void setRotation(const CTiglPoint& rotation);
    TIGL_EXPORT void setRotationType(ECPACSTranslationType rotationType);
    TIGL_EXPORT void setScaling(const CTiglPoint& scale);
    TIGL_EXPORT void setScalingType(ECPACSTranslationType scalingType);
    TIGL_EXPORT void setTransformationMatrix(const CTiglTransformation& matrix);
    
    TIGL_EXPORT CTiglPoint getTranslationVector() const;
    TIGL_EXPORT CTiglPoint getRotation() const;
    TIGL_EXPORT CTiglPoint getScaling() const;
    TIGL_EXPORT ECPACSTranslationType getTranslationType() const;
    TIGL_EXPORT ECPACSTranslationType getScalingType() const;
    TIGL_EXPORT ECPACSTranslationType getRotationType() const;
    TIGL_EXPORT CTiglTransformation getTransformationMatrix() const;

    // Helper function to set or reset a CCPACSTransformation.
    // This function will generate and set all the children uid based on the given uid
    // and set the transformation to the identity.
    TIGL_EXPORT void Init(const std::string& UID);
    
    /**
    * Reads in the transformation from a cpacs path
    * @param tixiHandle Handle to the xml document
    * @param transformationXPath XPath to the parent object
    */
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& transformationXPath) override;

private:
    void InvalidateImpl(const boost::optional<std::string>& source) const override;

    void updateMatrix(CTiglTransformation& cache) const;

    // caches the transformation created from scaling, rotation and translation
    Cache<CTiglTransformation, CCPACSTransformation> _transformationMatrix;

    // ABS_LOCAL per default only applies for translation, while scaling and rotation is global
    ECPACSTranslationType _scalingType = ABS_GLOBAL;
    ECPACSTranslationType _rotationType = ABS_GLOBAL;
};

} // namespace tigl
#endif // CCPACSTRANSFORMATION_H
