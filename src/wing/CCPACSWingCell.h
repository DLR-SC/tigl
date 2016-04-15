/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CCPACSWINGCELL_H
#define CCPACSWINGCELL_H

#include <iostream>
#include <vector>

#include "tixi.h"
#include "tigl_internal.h"
#include "CCPACSMaterial.h"

// [[CAS_AES]] added includes
// [[CAS_AES]] BEGIN
#include "CCPACSWingCellPositionChordwise.h"
#include "CCPACSWingCellPositionSpanwise.h"
#include "CCPACSWingStringer.h"
#include "CCPACSExplicitWingStringer.h"
#include "TopoDS_Shape.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS_Edge.hxx"
#include "gp_Pln.hxx"
// [[CAS_AES]] END


namespace tigl
{

// [[CAS_AES]] forward declarations
class CCPACSWingCells;
class CCPACSExplicitWingStringer;

// Typedef for a CCPACSExplicitWingStringer container to store the explicit wing stringer.
typedef std::vector<CCPACSExplicitWingStringer*> CCPACSExplicitWingStringerContainer;


class CCPACSWingCell
{
public:
    // [[CAS_AES]] added pointer to parent
    TIGL_EXPORT CCPACSWingCell(CCPACSWingCells* nParentCells);
    
    // [[CAS_AES]] added destructor
    TIGL_EXPORT virtual ~CCPACSWingCell();

    TIGL_EXPORT const std::string& GetUID() const;

    TIGL_EXPORT void Invalidate();

    
    // determines if a given eta xsi koordinate is inside this cell
    TIGL_EXPORT bool IsInside(double eta, double xsi) const;
    
    // determines if the cell defines a convex qudriangle or nor
    TIGL_EXPORT bool IsConvex() const;
    
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& cellXPath);
    
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & cellXPath);

    // get corner coordinates of cell
    TIGL_EXPORT void GetLeadingEdgeInnerPoint (double* eta, double* xsi) const;
    TIGL_EXPORT void GetLeadingEdgeOuterPoint (double* eta, double* xsi) const;
    TIGL_EXPORT void GetTrailingEdgeInnerPoint(double* eta, double* xsi) const;
    TIGL_EXPORT void GetTrailingEdgeOuterPoint(double* eta, double* xsi) const;
    
    // sets corner coordinates of cell
    TIGL_EXPORT void SetLeadingEdgeInnerPoint (double eta, double xsi);
    TIGL_EXPORT void SetLeadingEdgeOuterPoint (double eta, double xsi);
    TIGL_EXPORT void SetTrailingEdgeInnerPoint(double eta, double xsi);
    TIGL_EXPORT void SetTrailingEdgeOuterPoint(double eta, double xsi);

    TIGL_EXPORT CCPACSMaterial& GetMaterial();

    // [[CAS_AES]] Added setter for the leading edge positioning
    TIGL_EXPORT void SetPositionLeadingEdge(CCPACSWingCellPositionChordwise& nPosLE);

    // [[CAS_AES]] Added getter for the leading edge positioning
    TIGL_EXPORT CCPACSWingCellPositionChordwise& GetPositionLeadingEdge(void);

    // [[CAS_AES]] Added setter for the trailing edge positioning
    TIGL_EXPORT void SetPositionTrailingEdge(CCPACSWingCellPositionChordwise& nPosTE);

    // [[CAS_AES]] Added getter for the trailing edge positioning
    TIGL_EXPORT CCPACSWingCellPositionChordwise& GetPositionTrailingEdge(void);

    // [[CAS_AES]] Added setter for inner border positioning
    TIGL_EXPORT void SetPositionInnerBorder(CCPACSWingCellPositionSpanwise& nPosIB);

    // [[CAS_AES]] Added getter for inner border positioning
    TIGL_EXPORT CCPACSWingCellPositionSpanwise& GetPositionInnerBorder(void);

    // [[CAS_AES]] Added setter for outer border positioning
    TIGL_EXPORT void SetPositionOuterBorder(CCPACSWingCellPositionSpanwise& nPosOB);

    // [[CAS_AES]] Added getter for outer border positioning
    TIGL_EXPORT CCPACSWingCellPositionSpanwise& GetPositionOuterBorder(void);

    // [[CAS_AES]] get Pointer to parent cells element
    TIGL_EXPORT CCPACSWingCells* GetParentElement(void);

    // [[CAS_AES]] Getter for the stringer
    TIGL_EXPORT CCPACSWingStringer& GetStringer();

    // [[CAS_AES]] Returns whether a stringer definition exists or not
    TIGL_EXPORT bool HasStringer();

    // [[CAS_AES]] calculates the Eta/Xsi values of the Positions, if they are defined by rib or spar
    TIGL_EXPORT void CalcEtaXsi() const;

    // [[CAS_AES]] get if explicit stringer
    TIGL_EXPORT bool HasExplicitStringer();

    // [[CAS_AES]] get number of explicit stringer
    TIGL_EXPORT int NumberOfExplicitStringer();

    // [[CAS_AES]] get explicit stringer
    TIGL_EXPORT CCPACSExplicitWingStringer* GetExplicitStringerbyIndex(int);
    
    TIGL_EXPORT void Update();
    
    TIGL_EXPORT TopoDS_Shape GetCellSkinGeometry(bool transform=true);

    TIGL_EXPORT bool IsPartOfCell(TopoDS_Face);

    TIGL_EXPORT bool IsPartOfCell(TopoDS_Edge);

private:
    void Reset();
    
    void BuildSkinGeometry();

    // [[CAS_AES]] removed eta/xsi variables, are read from CCPACSWingCellPosition...
    std::string uid;
    CCPACSMaterial material;

    // [[CAS_AES]] added leading edge positioning
    mutable CCPACSWingCellPositionChordwise mPositionLeadingEdge;
    // [[CAS_AES]] added trailing edge positioning
    mutable CCPACSWingCellPositionChordwise mPositionTrailingEdge;
    // [[CAS_AES]] added inner border positioning
    mutable CCPACSWingCellPositionSpanwise mPositionInnerBorder;
    // [[CAS_AES]] added outer border positioning
    mutable CCPACSWingCellPositionSpanwise mPositionOuterBorder;

    // [[CAS_AES]] added stringer
    CCPACSWingStringer* stringer;
    // [[CAS_AES]] added explicit stringer
    CCPACSExplicitWingStringerContainer explicitStringers;
    
    bool geometryValid;
    TopoDS_Shape cellSkinGeometry;

    // [[CAS_AES]] added pointer to parent
    CCPACSWingCells* mParentCells;
    
    gp_Pln mCutPlaneLE, mCutPlaneTE, mCutPlaneIB, mCutPlaneOB;
    TopoDS_Shape mPlaneShapeLE, mPlaneShapeTE, mPlaneShapeIB, mPlaneShapeOB;
    gp_Pnt mPC1, mPC2, mPC3, mPC4;
    
    
};

namespace WingCellInternal
{

    struct Point2D
    {
        double x;
        double y;
    };

    // calculates crossproduct (p1-p3)x(p2-p3) (only "z"-value)
    TIGL_EXPORT double sign(Point2D p1, Point2D p2, Point2D p3);

    // calculates the area of a triangle
    TIGL_EXPORT double area(Point2D p1, Point2D p2, Point2D p3);

    // checks if point p is in triangle p1-p2-p3
    TIGL_EXPORT bool is_in_trian(Point2D p, Point2D p1, Point2D p2, Point2D p3);
}

} // namespace tigl

#endif // CCPACSWINGCELL_H
