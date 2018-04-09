/*
 * Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
 *
 * Created: 2014-01-28 Mark Geiger <Mark.Geiger@dlr.de>
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

#ifndef CCTiglControlSurfaceTransformation_H
#define CCTiglControlSurfaceTransformation_H

#include <string>

#include "CTiglError.h"
#include "tigl_internal.h"

#include "gp_Trsf.hxx"

namespace tigl
{

//@todo: small description of the class
class CTiglControlSurfaceTransformation
{
public:

    TIGL_EXPORT CTiglControlSurfaceTransformation(gp_Pnt hingePoint1 ,gp_Pnt hingePoint2,
                                                  gp_Pnt hingePoint1s, gp_Pnt hingePoint2s,
                                                  double rotationAlpha);

    TIGL_EXPORT gp_Trsf getTotalTransformation();
    TIGL_EXPORT gp_Trsf getTranslationTransformation();
    TIGL_EXPORT gp_Trsf getScaleTransformation();
    TIGL_EXPORT gp_Trsf getRotAlphaTransformation();
    TIGL_EXPORT gp_Trsf getRotPhiTransformation();
    TIGL_EXPORT gp_Trsf getRotThetaTransformation();
    TIGL_EXPORT gp_Trsf getToLocalTransformation();
    TIGL_EXPORT gp_Trsf getFromLocalTransformation();

    TIGL_EXPORT ~CTiglControlSurfaceTransformation();

private:
    gp_Trsf rotAlpha;
    gp_Trsf rotPhi;
    gp_Trsf rotTheta;
    gp_Trsf translation;
    gp_Trsf scale;
    gp_Trsf transformationToLocal;
    gp_Trsf transformationFromLocal;
    gp_Trsf totalTransformation;
};


} // end namespace tigl

#endif // CCTiglControlSurfaceTransformation_H
