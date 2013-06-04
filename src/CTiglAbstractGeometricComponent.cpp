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

#include <BRepBuilderAPI_Transform.hxx>

namespace tigl {

    // Constructor
    CTiglAbstractGeometricComponent::CTiglAbstractGeometricComponent(void)
        : mySymmetryAxis(TIGL_NO_SYMMETRY)
        , myUID("")
    {
        Reset();
    }

    void CTiglAbstractGeometricComponent::Reset(){
        SetUID("");
        mySymmetryAxis = TIGL_NO_SYMMETRY;
        transformation.SetIdentity();
        backTransformation.SetIdentity();
        translation = CTiglPoint(0.,0.,0.);
        scaling     = CTiglPoint(1.,1.,1.);
        rotation    = CTiglPoint(0.,0.,0.);
    }

    // Destructor
    CTiglAbstractGeometricComponent::~CTiglAbstractGeometricComponent(void)
    {
    }

    // Gets the component uid
    const std::string &CTiglAbstractGeometricComponent::GetUID(void)
    {
        return myUID;
    }

    // Sets the component uid
    void CTiglAbstractGeometricComponent::SetUID(const std::string& uid)
    {
        myUID = uid;
    }

    // Gets symmetry axis
    TiglSymmetryAxis CTiglAbstractGeometricComponent::GetSymmetryAxis(void)
    {
        return mySymmetryAxis;
    }

    // Gets symmetry axis
    void CTiglAbstractGeometricComponent::SetSymmetryAxis(const std::string& axis)
    {
        if (axis == "x-z-plane") {
            mySymmetryAxis = TIGL_X_Z_PLANE;
        } else if (axis == "x-y-plane") {
            mySymmetryAxis = TIGL_X_Y_PLANE;
        } else if (axis == "y-z-plane") {
            mySymmetryAxis = TIGL_Y_Z_PLANE;
        } else {
            mySymmetryAxis = TIGL_NO_SYMMETRY;
        }
    }

    // Returns a unique Hashcode for a specific geometric component
    int CTiglAbstractGeometricComponent::GetComponentHashCode(void)
    {
        TopoDS_Shape& loft = GetLoft();
        if(!loft.IsNull()){
            return loft.HashCode(2294967295);
        }
        else
            return 0;
    }

    CTiglTransformation CTiglAbstractGeometricComponent::GetTransformation(){
        return transformation;
    }

    CTiglPoint CTiglAbstractGeometricComponent::GetTranslation(){
        return translation;
    }

    void CTiglAbstractGeometricComponent::Translate(CTiglPoint trans){
        translation.x += trans.x;
        translation.y += trans.y;
        translation.z += trans.z;
    }

    TopoDS_Shape CTiglAbstractGeometricComponent::GetMirroredLoft(void){
        if(mySymmetryAxis == TIGL_NO_SYMMETRY){
            TopoDS_Shape nullShape;
            nullShape.Nullify();
            return  nullShape;
        }

        gp_Ax2 mirrorPlane;
        if(mySymmetryAxis == TIGL_X_Z_PLANE){
            mirrorPlane = gp_Ax2(gp_Pnt(0,0,0),gp_Dir(0.,1.,0.));
        }
        else if(mySymmetryAxis == TIGL_X_Y_PLANE){
            mirrorPlane = gp_Ax2(gp_Pnt(0,0,0),gp_Dir(0.,0.,1.));
        }
        else if(mySymmetryAxis == TIGL_Y_Z_PLANE){
            mirrorPlane = gp_Ax2(gp_Pnt(0,0,0),gp_Dir(1.,0.,0.));
        }

        gp_Trsf theTransformation;
        theTransformation.SetMirror(mirrorPlane);
        BRepBuilderAPI_Transform myBRepTransformation(GetLoft(), theTransformation);

        return myBRepTransformation.Shape();
    }

} // end namespace tigl
