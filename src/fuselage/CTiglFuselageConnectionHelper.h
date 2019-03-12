/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2018 Malo Drougard <malo.drougard@protonmail.com>
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

#ifndef TIGL_CTIGLFUSELAGECONNECTIONHELPER_H
#define TIGL_CTIGLFUSELAGECONNECTIONHELPER_H

#include "CTiglTransformation.h"
#include "CTiglFuselageConnection.h"

namespace tigl
{

/**
 * This class is a helper class for the CTiglFuselageConnection.
 * It's goal is to provide some usual operations performed on CTiglFuselageConnection class.
 */
class CTiglFuselageConnectionHelper
{
public:
    /**
     * This function return the transformation that the cpacs element of the given connection needs to have
     * its origin at the given position.
     * @remark the position is obtain by translation, so the scale or the normal of the profile does not change.
     */
    static CTiglTransformation
    GetTransformToPlaceConnectionOriginByTranslationAt(const CTiglFuselageConnection& connection,
                                                       const CTiglPoint& wantedOriginP);
};
}

#endif //TIGL_CTIGLFUSELAGECONNECTIONHELPER_H
