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

#ifndef CCPACSWINGSHELL_H
#define CCPACSWINGSHELL_H

#include "tigl_internal.h"
#include "CCPACSWingCells.h"
#include "CCPACSMaterial.h"

// [[CAS_AES]] added include for TiglLoftSide
#include "tigl.h"
// [[CAS_AES]] added include for parent reference
#include "TiglWingStructureReference.h"
// [[CAS_AES]] added include for stringer
#include "CCPACSWingStringer.h"

#include <gp_Vec.hxx>

#include <string>


namespace tigl 
{

class CCPACSWingShell
{
public:
    // [[CAS_AES]] added reference to parent element
    TIGL_EXPORT CCPACSWingShell(const TiglWingStructureReference& nWingStructureReference, TiglLoftSide side);

    // [[CAS_AES]] added destructor
    TIGL_EXPORT virtual ~CCPACSWingShell();

    TIGL_EXPORT void Reset();
    
    TIGL_EXPORT const std::string& GetUID() const;

    TIGL_EXPORT int GetCellCount() const;

    TIGL_EXPORT CCPACSWingCell& GetCell(int index);

    TIGL_EXPORT CCPACSMaterial& GetMaterial();
    
    // [[CAS_AES]] Returns the component segment this structure belongs to
    TIGL_EXPORT const TiglWingStructureReference& GetWingStructureReference(void) ;

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& shellXPath);

    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & shellDefinitionXPath);

    TIGL_EXPORT void Invalidate();
    TIGL_EXPORT bool IsValid() const;

    // [[CAS_AES]] Returns whether a stringer definition exists or not
    TIGL_EXPORT bool HasStringer() const;

    // [[CAS_AES]] Returns whether a stringer definition exists or not
    TIGL_EXPORT bool HasOverallStringer() const;

    // [[CAS_AES]] Getter for the stringer
    TIGL_EXPORT CCPACSWingStringer& GetStringer();

    // [[CAS_AES]] Getter for stringer wire geometry
    // use unsplitted Loft of the segment
    TIGL_EXPORT TopoDS_Shape GetStringerGeometry(bool relativeToWing = false);

    // [[CAS_AES]] Getter for split geometry
    TIGL_EXPORT TopoDS_Shape GetSplitGeometry();

    // [[CAS_AES]] Cut wingSegment Loft with the Stringer lines
    TIGL_EXPORT TopoDS_Shape GetSplittedWithStringerGeometry(TopoDS_Shape nSegmentLoft);

    // [[CAS_AES]] returns if a cell has stringers
    TIGL_EXPORT bool HasStringerOfCell();
    // [[CAS_AES]] returns if a cell has stringers
    TIGL_EXPORT bool HasStringerOfCell(int index);

    // [[CAS_AES]] returns the stringer of a cell element
    TIGL_EXPORT CCPACSWingStringer& GetStringerOfCell(int);

    // [[CAS_AES]] for stringer meshing
    TIGL_EXPORT int GetNumberOfStringer();

    // [[CAS_AES]] added method
    TIGL_EXPORT TopoDS_Shape GetStringerEdges(int);
    // [[CAS_AES]] added method
    TIGL_EXPORT std::string GetStringerStructuralUId(int);

    TIGL_EXPORT CTiglAbstractWingStringer* GetStringerByIndex(int nIndex);

    // [[CAS_AES]] added method
    TIGL_EXPORT bool SparSegmentsTest(gp_Ax1, gp_Pnt, TopoDS_Shape, bool doWingTrafo = false);

    // [[CAS_AES]] added method
    TIGL_EXPORT void Update();

    TIGL_EXPORT TiglLoftSide GetLoftSide() const;

protected:

    // [[CAS_AES]] added method
    void BuildStringerGeometry(CCPACSWingCell* nCell = 0);

    // [[CAS_AES]] added method to build explicit stringer
    void BuildExplicitStringerGeometry(CCPACSWingCell* nCell);

private:

    // [[CAS_AES]] added method to return the stringer angle reference as method
    gp_Vec GetAngleReference(std::string);


private:
    // Information whether the shell is located on the upper or lower side of the loft
    TiglLoftSide side;
    CCPACSWingCells cells;
    CCPACSMaterial material;
    //@todo stringers

    // [[CAS_AES]] added uid
    std::string uid;
    // [[CAS_AES]] added stringer
    CCPACSWingStringer* stringer;
    // [[CAS_AES]] added stringer edges
    TopoDS_Shape stringerEdges;
    // [[CAS_AES]] added cutting planes
    TopoDS_Shape mCuttingPlanes;
    // [[CAS_AES]] added reference to parent
    TiglWingStructureReference wingStructureReference;

    bool isvalid;
};

} // namespace tigl

#endif // CCPACSWINGSHELL_H
