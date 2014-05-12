/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
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

#ifndef ORTHOMANIPULATOR_H
#define ORTHOMANIPULATOR_H

#include <osgGA/OrbitManipulator>

class OrthoManipulator : public osgGA::OrbitManipulator
{
public:
    OrthoManipulator(osg::Camera* cam);
    bool performMovementMiddleMouseButton( const double eventTimeDelta, const double dx, const double dy );
    bool performMovementRightMouseButton( const double eventTimeDelta, const double /*dx*/, const double dy );
    void panModel( const float dx, const float dy, const float dz );
    void zoomModel( const float dy, bool pushForwardIfNeeded);
    
private:
    osg::Camera* _camera;
};

#endif // ORTHOMANIPULATOR_H
