/*
* Copyright (C) 2016 German Aerospace Center (DLR/SC)
*
* Created: 2016-02-24 Martin Siggel <martin.siggel@dlr.de>
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

#include "CTiglControlSurfaceBorderCoordinateSystem.h"

#include <gp_Ax3.hxx>
#include <gp_Trsf.hxx>

namespace tigl
{

CTiglControlSurfaceBorderCoordinateSystem::CTiglControlSurfaceBorderCoordinateSystem(gp_Pnt le, gp_Pnt te, gp_Vec upDir)
    : _le(le), _te(te), _ydir(upDir)
{
    _xdir = _te.XYZ()-_le.XYZ();
    _xdir.Normalize();
    
    _zdir = _xdir.Crossed(upDir.Normalized());
    _zdir.Normalize();
    
    _ydir = _zdir.Crossed(_xdir);
}

gp_Vec CTiglControlSurfaceBorderCoordinateSystem::getNormal() const
{
    return _zdir;
}

gp_Pnt CTiglControlSurfaceBorderCoordinateSystem::getLe() const
{
    return _le;
}

gp_Pnt CTiglControlSurfaceBorderCoordinateSystem::getTe() const
{
    return _te;
}

gp_Vec CTiglControlSurfaceBorderCoordinateSystem::getXDir() const
{
    return _xdir;
}

gp_Vec CTiglControlSurfaceBorderCoordinateSystem::getYDir() const
{
    return _ydir;
}

gp_Pln CTiglControlSurfaceBorderCoordinateSystem::getPlane() const
{
    gp_Pln plane(gp_Ax3(_le, getNormal().XYZ(), getXDir().XYZ()));
    return plane;
}

gp_Trsf CTiglControlSurfaceBorderCoordinateSystem::globalTransform() const
{
    gp_Trsf t;
    t.SetTransformation(gp_Ax3(_le, getNormal(), getXDir()), 
                        gp_Ax3(gp_Pnt(0,0,0), gp_Vec(0,0,1), gp_Vec(1,0,0)));
    return t;
}

} // namespace tigl

