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

#include "generated/CPACSWingCell.h"
#include "CCPACSMaterial.h"

namespace tigl
{

class CCPACSWingCell : public generated::CPACSWingCell
{
public:
    TIGL_EXPORT CCPACSWingCell();

    // determines if a given eta xsi koordinate is inside this cell
    TIGL_EXPORT bool IsInside(double eta, double xsi) const;
    
    // determines if the cell defines a convex qudriangle or nor
    TIGL_EXPORT bool IsConvex() const;
    
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& cellXPath);

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
    
    TIGL_EXPORT const CCPACSMaterial& GetMaterial();
private:
    void reset();
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
