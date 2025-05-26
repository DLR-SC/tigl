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

#ifndef CCPACSTransformationSE3_H
#define CCPACSTransformationSE3_H

#include "generated/CPACSTransformationSE3.h"

#include "CTiglTransformation.h"
#include "ECPACSTranslationType.h"
#include "Cache.h"

namespace tigl
{

class CCPACSTransformationSE3 : public generated::CPACSTransformationSE3
{
public:
    TIGL_EXPORT CCPACSTransformationSE3(CCPACSComponent* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSTransformationSE3(CCPACSElementGeometryAddtionalPart* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT void reset();

    TIGL_EXPORT CCPACSTransformationSE3& operator=(const CCPACSTransformationSE3& trafo);

    TIGL_EXPORT void setTranslation(const CTiglPoint& translation);
    TIGL_EXPORT void setTranslation(const CTiglPoint& translation, ECPACSTranslationType);
    TIGL_EXPORT void setRotation(const CTiglPoint& rotation);
    TIGL_EXPORT void setRotationType(ECPACSTranslationType rotationType);
    TIGL_EXPORT void setTransformationMatrix(const CTiglTransformation& matrix);

    TIGL_EXPORT CTiglPoint getTranslationVector() const;
    TIGL_EXPORT CTiglPoint getRotation() const;
    TIGL_EXPORT ECPACSTranslationType getTranslationType() const;
    TIGL_EXPORT ECPACSTranslationType getRotationType() const;
    TIGL_EXPORT CTiglTransformation getTransformationMatrix() const;

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
    Cache<CTiglTransformation, CCPACSTransformationSE3> _transformationMatrix;

    // ABS_LOCAL per default only applies for translation, while rotation is global
    ECPACSTranslationType _rotationType = ABS_GLOBAL;
};

} // namespace tigl
#endif // CCPACSTransformationSE3_H
