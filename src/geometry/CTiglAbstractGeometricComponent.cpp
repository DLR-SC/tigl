/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  Partial implementation of the ITiglGeometricComponent interface.
*/

#include "CTiglAbstractGeometricComponent.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TiglSymmetryAxis.h"
#include "CCPACSTransformation.h"
#include "CNamedShape.h"

// OCCT defines
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

namespace tigl
{
CTiglAbstractGeometricComponent::CTiglAbstractGeometricComponent()
    : loft(*this, &CTiglAbstractGeometricComponent::BuildLoft)
    , bounding_box(*this, &CTiglAbstractGeometricComponent::CalcBoundingBox)
{
}

void CTiglAbstractGeometricComponent::Reset() const {
    bounding_box.clear();
    loft.clear();
}

TiglSymmetryAxis CTiglAbstractGeometricComponent::GetSymmetryAxis() const
{
    return TIGL_NO_SYMMETRY;
}

PNamedShape CTiglAbstractGeometricComponent::GetLoft() const
{
    return *loft;
}

Bnd_Box const& CTiglAbstractGeometricComponent::GetBoundingBox() const
{
    return *bounding_box;
}

PNamedShape CTiglAbstractGeometricComponent::GetMirroredLoft() const
{
    const TiglSymmetryAxis& symmetryAxis = GetSymmetryAxis();
    if (symmetryAxis == TIGL_NO_SYMMETRY) {
        return PNamedShape();
    }

    CTiglTransformation trafo;
    if (symmetryAxis == TIGL_X_Z_PLANE) {
        trafo.AddMirroringAtXZPlane();
    }
    else if (symmetryAxis == TIGL_X_Y_PLANE) {
        trafo.AddMirroringAtXYPlane();
    }
    else if (symmetryAxis == TIGL_Y_Z_PLANE) {
        trafo.AddMirroringAtYZPlane();
    }

    PNamedShape mirroredShape = trafo.Transform(GetLoft());

    std::string mirrorName = mirroredShape->Name();
    mirrorName += "M";
    std::string mirrorShortName = mirroredShape->ShortName();
    mirrorShortName += "M";
    mirroredShape->SetName(mirrorName.c_str());
    mirroredShape->SetShortName(mirrorShortName.c_str());
    return mirroredShape;
}

bool CTiglAbstractGeometricComponent::GetIsOn(const gp_Pnt& pnt) const
{
    const TopoDS_Shape segmentShape = GetLoft()->Shape();

    // fast check with bounding box
    Bnd_Box boundingBox;
    BRepBndLib::Add(segmentShape, boundingBox);

    Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
    boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    if (pnt.X() < xmin || pnt.X() > xmax ||
        pnt.Y() < ymin || pnt.Y() > ymax ||
        pnt.Z() < zmin || pnt.Z() > zmax) {

        return false;
    }

    double tolerance = 0.03; // 3cm

    BRepClass3d_SolidClassifier classifier;
    classifier.Load(segmentShape);
    classifier.Perform(pnt, tolerance);
    if ((classifier.State() == TopAbs_IN) || (classifier.State() == TopAbs_ON)) {
        return true;
    }
    else {
        return false;
    }
}

bool CTiglAbstractGeometricComponent::GetIsOnMirrored(const gp_Pnt& pnt) const
{
    const TiglSymmetryAxis& symmetryAxis = GetSymmetryAxis();
    if (symmetryAxis == TIGL_NO_SYMMETRY) {
        return false;
    }

    gp_Pnt mirroredPnt(pnt);
    if (symmetryAxis == TIGL_X_Z_PLANE) {
        mirroredPnt.SetY(-mirroredPnt.Y());
    }
    else if (symmetryAxis == TIGL_X_Y_PLANE) {
        mirroredPnt.SetZ(-mirroredPnt.Z());
    }
    else if (symmetryAxis == TIGL_Y_Z_PLANE) {
        mirroredPnt.SetX(-mirroredPnt.X());
    }
    
    return GetIsOn(mirroredPnt);
}

void CTiglAbstractGeometricComponent::BuildLoft(PNamedShape& cache) const
{
    cache = BuildLoft();
}

void CTiglAbstractGeometricComponent::CalcBoundingBox(Bnd_Box& bb) const
{
    BRepBndLib::Add(loft->get()->Shape(), bb);
}

} // end namespace tigl
