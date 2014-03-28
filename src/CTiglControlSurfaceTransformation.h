/*
 * Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
 *
 * Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
 * Changed: $Id$
 *
 * Version: $Revision$
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
 * @brief  Implementation of ..
 */

#ifndef CCTiglControlSurfaceTransformation_H
#define CCTiglControlSurfaceTransformation_H

#include <string>

#include "CTiglError.h"

#include "gp_Trsf.hxx"

namespace tigl {

class CTiglControlSurfaceTransformation
{

private:

    gp_Trsf rotAlpha;
    gp_Trsf rotPhi;
    gp_Trsf rotTheta;
    gp_Trsf translation;
    gp_Trsf scale;
    gp_Trsf transformationToLocal;
    gp_Trsf transformationFromLocal;
    gp_Trsf totalTransformation;



public:

    CTiglControlSurfaceTransformation(gp_Pnt hingePoint1 ,gp_Pnt hingePoint2,
                                      gp_Pnt hingePoint1s, gp_Pnt hingePoint2s,
                                      double rotationAlpha);

    gp_Trsf getTotalTransformation();
    gp_Trsf getTranslationTransformation();
    gp_Trsf getScaleTransformation();
    gp_Trsf getRotAlphaTransformation();
    gp_Trsf getRotPhiTransformation();
    gp_Trsf getRotThetaTransformation();
    gp_Trsf getToLocalTransformation();
    gp_Trsf getFromLocalTransformation();

    ~CTiglControlSurfaceTransformation();
};


} // end namespace tigl

#endif // CCTiglControlSurfaceTransformation_H
