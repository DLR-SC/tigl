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
* @brief  Partial implementation of the ITiglGeometricComponent interface.
*/

#include "CTiglAbstractGeometricComponent.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TiglSymmetryAxis.h"
#include "CCPACSTransformation.h"

// OCCT defines
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

namespace tigl
{

// Constructor
CTiglAbstractGeometricComponent::CTiglAbstractGeometricComponent(CCPACSTransformation& trans, TiglSymmetryAxis& symmetryAxis)
    : transformation(trans), symmetryAxis(symmetryAxis) {}

CTiglAbstractGeometricComponent::CTiglAbstractGeometricComponent(CCPACSTransformation& trans, boost::optional<TiglSymmetryAxis>& symmetryAxis)
    : transformation(trans), symmetryAxis(symmetryAxis) {}

void CTiglAbstractGeometricComponent::Reset()
{
    SetUID("");
    SetSymmetryAxis(TiglSymmetryAxis::TIGL_NO_SYMMETRY);
    transformation.reset();
}

TiglSymmetryAxis CTiglAbstractGeometricComponent::GetSymmetryAxis() {
	struct Visitor : boost::static_visitor<TiglSymmetryAxis> {
		TiglSymmetryAxis operator()(const TiglSymmetryAxis& s) {
			return s;
		}
		TiglSymmetryAxis operator()(const boost::optional<TiglSymmetryAxis>& s) {
			return *s;
		}
	} visitor;
	return symmetryAxis.apply_visitor(visitor);
}

void CTiglAbstractGeometricComponent::SetSymmetryAxis(const TiglSymmetryAxis& axis) {
	struct Visitor : boost::static_visitor<> {
		Visitor(const TiglSymmetryAxis& axis)
			: axis(axis) {}
		void operator()(TiglSymmetryAxis& s) {
			s = axis;
		}
		void operator()(boost::optional<TiglSymmetryAxis>& s) {
			s = axis;
		}
	private:
		const TiglSymmetryAxis& axis;
	} visitor(axis);
	symmetryAxis.apply_visitor(visitor);
}

std::string CTiglAbstractGeometricComponent::GetSymmetryAxisString() {
    return TiglSymmetryAxisToString(GetSymmetryAxis());
}

void CTiglAbstractGeometricComponent::SetSymmetryAxis(const std::string& axis) {
    SetSymmetryAxis(stringToTiglSymmetryAxis(axis));
}

CTiglTransformation CTiglAbstractGeometricComponent::GetTransformation() const
{
    return transformation.getTransformationMatrix();
}

CTiglPoint CTiglAbstractGeometricComponent::GetTranslation() const
{
    return transformation.getTranslationVector();
}

ECPACSTranslationType CTiglAbstractGeometricComponent::GetTranslationType(void) const
{
    return transformation.getTranslationType();
}

CTiglPoint CTiglAbstractGeometricComponent::GetRotation() const
{
    return transformation.getRotation();
}

CTiglPoint CTiglAbstractGeometricComponent::GetScaling() const
{
    return transformation.getScaling();
}

void CTiglAbstractGeometricComponent::Translate(CTiglPoint trans)
{
    CTiglPoint newTrans(transformation.getTranslationVector());
    newTrans += trans;
    transformation.setTranslation(newTrans, transformation.getTranslationType());
    transformation.updateMatrix();
}

PNamedShape CTiglAbstractGeometricComponent::GetLoft()
{
    if (!loft) {
#ifdef DEBUG
        LOG(INFO) << "Building loft " << GetUID();
#endif
        loft = BuildLoft();
    }
    return loft;
}

PNamedShape CTiglAbstractGeometricComponent::GetMirroredLoft()
{
    const auto symmetryAxis = GetSymmetryAxis();
    if (symmetryAxis == TIGL_NO_SYMMETRY) {
        PNamedShape nullShape;
        nullShape.reset();
        return nullShape;
    }

    gp_Ax2 mirrorPlane;
    if (symmetryAxis == TIGL_X_Z_PLANE) {
        mirrorPlane = gp_Ax2(gp_Pnt(0,0,0),gp_Dir(0.,1.,0.));
    }
    else if (symmetryAxis == TIGL_X_Y_PLANE) {
        mirrorPlane = gp_Ax2(gp_Pnt(0,0,0),gp_Dir(0.,0.,1.));
    }
    else if (symmetryAxis == TIGL_Y_Z_PLANE) {
        mirrorPlane = gp_Ax2(gp_Pnt(0,0,0),gp_Dir(1.,0.,0.));
    }

    gp_Trsf theTransformation;
    theTransformation.SetMirror(mirrorPlane);
    BRepBuilderAPI_Transform myBRepTransformation(GetLoft()->Shape(), theTransformation);
    std::string mirrorName = GetLoft()->Name();
    mirrorName += "M";
    std::string mirrorShortName = GetLoft()->ShortName();
    mirrorShortName += "M";
    TopoDS_Shape mirroredShape = myBRepTransformation.Shape();
    
    PNamedShape mirroredPNamedShape(new CNamedShape(*GetLoft()));
    mirroredPNamedShape->SetShape(mirroredShape);
    mirroredPNamedShape->SetName(mirrorName.c_str());
    mirroredPNamedShape->SetShortName(mirrorShortName.c_str());
    return mirroredPNamedShape;
}

bool CTiglAbstractGeometricComponent::GetIsOn(const gp_Pnt& pnt) 
{
    const TopoDS_Shape& segmentShape = GetLoft()->Shape();

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

bool CTiglAbstractGeometricComponent::GetIsOnMirrored(const gp_Pnt& pnt) 
{
    const auto symmetryAxis = GetSymmetryAxis();
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

} // end namespace tigl
