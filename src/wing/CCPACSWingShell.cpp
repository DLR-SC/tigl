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

#include "CCPACSWingShell.h"

#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TixiSaveExt.h"
// [[CAS_AES]] added includes
// [[CAS_AES]] BEGIN
#include "CTiglCommon.h"
#include "CCPACSExplicitWingStringer.h"
#include "CCPACSWingCell.h"
#include "CCPACSWing.h"
#include "CCPACSWingSpars.h"
#include "CCPACSWingSparSegments.h"
#include "CCPACSWingSparSegment.h"
#include "CCPACSWingSparPositionUIDs.h"
#include "CCPACSWingSparPosition.h"

#include <Bnd_Box.hxx>
#include <BRep_Builder.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepLib_MakeFace.hxx>
#include <BRepTools.hxx>
#include <Geom_Surface.hxx>
#include <gp_Ax3.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt2d.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <TopoDS_Compound.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
// [[CAS_AES]] END

namespace tigl 
{

// [[CAS_AES]] added reference to parent element
// [[CAS_AES]] added initialization of stringer and cell
CCPACSWingShell::CCPACSWingShell(const TiglWingStructureReference& nWingStructureReference, TiglLoftSide side)
: wingStructureReference(nWingStructureReference),
  side(side),
  stringer(NULL),
  cells(this)
{
    Reset();
}

// [[CAS_AES]] added destructor
CCPACSWingShell::~CCPACSWingShell()
{
    Reset();
}

void CCPACSWingShell::Reset()
{
    cells.Reset();
    // [[CAS_AES]] free memory for stringer
    if (stringer != NULL) {
        delete stringer;
        stringer = NULL;
    }
    Invalidate();
}

const std::string& CCPACSWingShell::GetUID() const
{
    return uid;
}

int CCPACSWingShell::GetCellCount() const
{
    return cells.GetCellCount();
}

CCPACSWingCell& CCPACSWingShell::GetCell(int index)
{
    return cells.GetCell(index);
}

CCPACSMaterial& CCPACSWingShell::GetMaterial()
{
    return material;
}

// [[CAS_AES]] Returns the component segment this structure belongs to
const TiglWingStructureReference& CCPACSWingShell::GetWingStructureReference(void)
{
    return wingStructureReference;
}

void CCPACSWingShell::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string &shellXPath)
{
    Reset();
    
    // check path
    if ( tixiCheckElement(tixiHandle, shellXPath.c_str()) != SUCCESS) {
        LOG(ERROR) << "Wing Shell " << shellXPath << " not found in CPACS file!" << std::endl;
        return;
    }

    // [[CAS_AES]] Get UID
    char* ptrUID = NULL;
    if (tixiGetTextAttribute(tixiHandle, shellXPath.c_str(), "uID", &ptrUID) == SUCCESS) {
        uid = ptrUID;
    }

    // [[CAS_AES]] Get subelement stringer
    int count;
    ReturnCode tixiRet = tixiGetNamedChildrenCount(tixiHandle, shellXPath.c_str(), "stringer", &count);
    if (tixiRet == SUCCESS && count == 1) {
        stringer = new CCPACSWingStringer(this);
        std::string tempString = shellXPath + "/stringer";
        stringer->ReadCPACS(tixiHandle, tempString);
    }

    // read cell data
    std::string cellpath = shellXPath + "/cells";
    if (tixiCheckElement(tixiHandle, cellpath.c_str()) == SUCCESS) {
        cells.ReadCPACS(tixiHandle, cellpath.c_str());
    }
    
    // read material
    std::string materialString;
    materialString = shellXPath + "/skin/material";
    if ( tixiCheckElement(tixiHandle, materialString.c_str()) == SUCCESS) {
        material.ReadCPACS(tixiHandle, materialString.c_str());
    }
    else {
        // @todo: should that be an error?
        LOG(WARNING) << "No material definition found for shell " << shellXPath;
    }
    
    // [[CAS_AES]] commented out validation flag since geometry is not generated yet
    //isvalid = true;
}

// Write CPACS segment elements
void CCPACSWingShell::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& shellDefinitionXPath)
{
    std::string elementPath;
    
    TixiSaveExt::TixiSaveElement(tixiHandle, shellDefinitionXPath.c_str(), "skin");

    if (stringer != NULL) {
        TixiSaveExt::TixiSaveElement(tixiHandle, shellDefinitionXPath.c_str(), "stringer");
        elementPath = shellDefinitionXPath + "/stringer";
        stringer->WriteCPACS(tixiHandle, elementPath);
    }

    if (material.GetUID() != "UID_NOTSET") {
        elementPath = shellDefinitionXPath + "/skin";
        TixiSaveExt::TixiSaveElement(tixiHandle, elementPath.c_str(), "material");
        elementPath = shellDefinitionXPath + "/skin/material";
        material.WriteCPACS(tixiHandle, elementPath);
    }
    
    if (cells.GetCellCount() > 0) {
        TixiSaveExt::TixiSaveElement(tixiHandle, shellDefinitionXPath.c_str(), "cells");
        elementPath = shellDefinitionXPath + "/cells";
        cells.WriteCPACS(tixiHandle, elementPath);
    }
}

void CCPACSWingShell::Invalidate()
{
    isvalid = false;
    cells.Invalidate();
}

bool CCPACSWingShell::IsValid() const
{
    return isvalid;
}

// [[CAS_AES]] Returns whether a stringer definition exists or not
bool CCPACSWingShell::HasStringer() const
{
    if (stringer != 0) {
        return true;
    }

    for (int i = 1; i <= cells.GetCellCount(); i++) {
        if (cells.GetCell(i).HasStringer()) {
            return true;
        }

        if (cells.GetCell(i).HasExplicitStringer()) {
            return true;
        }
    }

    return false;
}

    // [[CAS_AES]] Returns whether a stringer definition exists or not
TIGL_EXPORT bool CCPACSWingShell::HasOverallStringer() const
{
    return (stringer != 0);
}


// [[CAS_AES]] Getter for the stringer
CCPACSWingStringer& CCPACSWingShell::GetStringer()
{
    if (stringer == NULL) {
        LOG(ERROR) << "Error: No stringer are existing in CCPACSWingShell::GetStringer()!" << std::endl;
        throw CTiglError("Error: No stringer are existing in CCPACSWingShell::GetStringer()!");
    }

    return *stringer;
}

// [[CAS_AES]] Getter for stringer wire geometry
TopoDS_Shape CCPACSWingShell::GetStringerGeometry(bool relativeToWing)
{
    if (!isvalid) {
        Update();
    }

    if (!relativeToWing) {
        return wingStructureReference.GetWing().GetWingTransformation().Transform(stringerEdges);
    }
    else {
        return stringerEdges;
    }
}

// [[CAS_AES]] Getter for split geometry
TopoDS_Shape CCPACSWingShell::GetSplitGeometry()
{
    if (!isvalid) {
        Update();
    }

    return mCuttingPlanes;
}

// [[CAS_AES]] Cut wingSegment Loft with the Stringer lines
TopoDS_Shape CCPACSWingShell::GetSplittedWithStringerGeometry(TopoDS_Shape nSegmentLoft)
{
// update Stringer geometry
    if (!isvalid) {
        Update();
    }

    return CTiglCommon::splitShape(nSegmentLoft, mCuttingPlanes);
}

// [[CAS_AES]] returns if a cell has stringers
bool CCPACSWingShell::HasStringerOfCell()
{
    if (cells.GetCellCount() <= 0) {
        return false;
    }

    for (int i = 1; i <= cells.GetCellCount(); i++) {
        if (cells.GetCell(i).HasStringer()) {
            return true;
        }
    }

    return false;
}

// [[CAS_AES]] returns if a cell has stringers
bool CCPACSWingShell::HasStringerOfCell(int index)
{
    return cells.GetCell(index).HasStringer();
}

// [[CAS_AES]] returns the stringer of a cell element
CCPACSWingStringer& CCPACSWingShell::GetStringerOfCell(int index)
{
    return cells.GetCell(index).GetStringer();
}

// [[CAS_AES]] for stringer meshing
int CCPACSWingShell::GetNumberOfStringer()
{
    int i = 0;

    if (stringer != 0) {
        i++;
    }

    for (int j = 1; j <= GetCellCount(); j++ ) {
        if (cells.GetCell(j).HasStringer()) {
            i++;
        }

        if (cells.GetCell(j).HasExplicitStringer()) {
            i += cells.GetCell(j).NumberOfExplicitStringer();
        }
    }

    return i;
}

// [[CAS_AES]] added method
TopoDS_Shape CCPACSWingShell::GetStringerEdges(int nIndex)
{
    if (stringer != 0 && nIndex == 1) {
        return wingStructureReference.GetWing().GetWingTransformation().Transform(stringer->GetStringerGeometry());
    }

    if (stringer != 0 && nIndex != 1) {
        nIndex--;
    }

    for (int j = 1; j <= GetCellCount(); j++ ) {
        if (j == nIndex && cells.GetCell(j).HasStringer()) {
            return wingStructureReference.GetWing().GetWingTransformation().Transform(cells.GetCell(j).GetStringer().GetStringerGeometry());
        }

        for (int k = 1; k <= cells.GetCell(j).NumberOfExplicitStringer(); k++) {
            if ((((j - 1) + k) == nIndex )) {
                return wingStructureReference.GetWing().GetWingTransformation().Transform(cells.GetCell(j).GetExplicitStringerbyIndex(k)->GetStringerGeometry());
            }
        }
    }

    LOG(ERROR) << "Error during Mesh generation:\nStringer geometry is not existing." << std::endl;
    throw CTiglError("Error during Mesh generation:\nStringer geometry is not existing.");
}

// [[CAS_AES]] added method
std::string CCPACSWingShell::GetStringerStructuralUId(int nIndex)
{
    if (stringer != 0 && nIndex == 1) {
        return stringer->GetStringerStructureUID();
    }

    if (stringer != 0 && nIndex != 1) {
        nIndex--;
    }

    for (int j = 1; j <= GetCellCount(); j++ ) {
        if (j == nIndex && cells.GetCell(j).HasStringer()) {
            return cells.GetCell(j).GetStringer().GetStringerStructureUID();
        }

        for (int k = 1; k <= cells.GetCell(j).NumberOfExplicitStringer(); k++ ) {
            if (((j - 1) + k) == nIndex) {
                return cells.GetCell(j).GetExplicitStringerbyIndex(k)->GetStringerStructureUID();
            }
        }
    }
    
    LOG(ERROR) << "Error during Mesh generation:\nStringer structural UID is not existing." << std::endl;
    throw CTiglError("Error during Mesh generation:\nStringer structural UID is not existing.");

}

CTiglAbstractWingStringer* CCPACSWingShell::GetStringerByIndex(int nIndex)
{
    if (stringer != 0 && nIndex == 1) {
        return stringer;
    }

    if (stringer != 0 && nIndex != 1) {
        nIndex--;
    }

    for (int j = 1; j <= GetCellCount(); j++ ) {
        if (j == nIndex && cells.GetCell(j).HasStringer()) {
            return &cells.GetCell(j).GetStringer();
        }

        for (int k = 1; k <= cells.GetCell(j).NumberOfExplicitStringer(); k++ ) {
            if (((j - 1) + k) == nIndex) {
                return cells.GetCell(j).GetExplicitStringerbyIndex(k);
            }
        }
    }

    LOG(ERROR) << "Error during Mesh generation:\nStringer not found by index." << std::endl;
    throw CTiglError("Error during Mesh generation:\nStringer not found by index.");
}

// [[CAS_AES]] added method
void CCPACSWingShell::Update()
{
    if (isvalid) {
        return;
    }

    stringerEdges.Nullify();
    mCuttingPlanes.Nullify();

    if (stringer != 0) {
        BuildStringerGeometry();
    }

    for (int i = 1; i <= cells.GetCellCount(); i++) {
        if (cells.GetCell(i).HasStringer()) {
            BuildStringerGeometry(&cells.GetCell(i));
        }

        if (cells.GetCell(i).HasExplicitStringer()) {
            BuildExplicitStringerGeometry(&cells.GetCell(i));
        }
    }

    isvalid = true;
}

TiglLoftSide CCPACSWingShell::GetLoftSide() const
{
    return side;
}

// [[CAS_AES]] added method
void CCPACSWingShell::BuildStringerGeometry(CCPACSWingCell* nCell) 
{

// Variable declaration
    TopoDS_Compound compound;
    TopoDS_Compound CCuttingCompound;
    TopoDS_Compound stringerCompound;
    TopoDS_Compound currentCompound;
    BRep_Builder builder;
// initialise compound
    builder.MakeCompound(compound);
    builder.MakeCompound(CCuttingCompound);
    builder.MakeCompound(stringerCompound);
    builder.MakeCompound(currentCompound);

// There can be another cell on the shell
    if (!stringerEdges.IsNull()) {
        builder.Add(compound, stringerEdges);
    }
    if (!stringerEdges.IsNull()) {
        builder.Add(CCuttingCompound, mCuttingPlanes);
    }


// variable for uv bounds
    double u_min=0, u_max=0, v_min=0, v_max=0;
// limits the available length for pitch and stringer angle calculation with the xsi coordinate of the reference point of the stringer
    double pitchScale = 0.0;

// verify if the stringer definition is for the comlete shell or a cell
    bool cell;
    if (nCell == 0) {
        cell = false;
    }
    else {
        cell = true;
    }

// define stringer construction variables
    int nrStr = 1;
    double pitch = 0.;
    gp_Pnt2d pRef2D(0.0,0.0);
    double stringerAngle = 0.;

    // determine diagonal vector of loft bounding box (e.g. used for size of cut faces)
    Bnd_Box boundingBox;
    BRepBndLib::Add(wingStructureReference.GetUpperShape(), boundingBox);
    BRepBndLib::Add(wingStructureReference.GetLowerShape(), boundingBox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    gp_Vec diagonal(xmax-xmin, ymax-ymin, zmax-zmin);
    Standard_Real bboxSize = diagonal.Magnitude();

    std::string angleRef;

// Step 1 : get the stringer definition from the cell ...
    if (cell) {
    
    // if a cell is defined with ribs and spars, the eta xsi values are calculated
        nCell->CalcEtaXsi();

        nrStr = nCell->GetStringer().GetNumOfStringers();
        pitch = nCell->GetStringer().GetPitch();
        stringerAngle = -(nCell->GetStringer().GetAngle()) * (M_PI / 180);
        angleRef = nCell->GetStringer().GetAngleReference();

    }
// ... or directly from the shell
    else {

        nrStr = stringer->GetNumOfStringers();
        pitch = stringer->GetPitch();
        stringerAngle = -(stringer->GetAngle()) * (M_PI / 180);

        pRef2D = gp_Pnt2d(stringer->GetEta(), stringer->GetXsi());

        pitchScale = 1 - stringer->GetXsi();

        angleRef = stringer->GetAngleReference();

    }

// Step 2:  Get the x and z reference direction for the stringer cutting planes

    gp_Pnt p1 = wingStructureReference.GetLeadingEdgePoint(0);
    gp_Pnt p2 = wingStructureReference.GetLeadingEdgePoint(1);
    gp_Vec yRefDir(p1,p2);
    yRefDir.Normalize();

    // create a reference direction without sweep angle
    gp_Pnt p2stern = gp_Pnt(p1.X(), p2.Y(), p2.Z());
    gp_Vec yRefDirStern(p1,p2stern);
    yRefDirStern.Normalize();

    double sweepAngle = yRefDir.Angle(yRefDirStern);

    if (p2.X() < p1.X()) {
        sweepAngle = -sweepAngle;
    }

    CCPACSWing& CW = wingStructureReference.GetWing();
    gp_Pnt p3(0.,0.,0.);
    gp_Pnt p4(0.,0.,1.);
    gp_Pnt p7(0.,1.,0.);

    gp_Vec globalYDir(p3,p7);
    globalYDir.Normalize();

//     Step 3: creation of a Component segment midplane face
// It is necessary to seperate between the upper and the lower surface
    gp_Pnt p5 = wingStructureReference.GetTrailingEdgePoint(0);
    gp_Pnt p6 = wingStructureReference.GetTrailingEdgePoint(1);

    gp_Vec xRefDir(p1,p5);
    xRefDir.Normalize();

    gp_Vec zRefDir(-yRefDirStern^xRefDir);
//     gp_Vec zRefDir(-globalYDir^xRefDir);
    zRefDir.Normalize();

//     Reference root length for the pitch / NrofStringers calculation
    gp_Vec vRoot(p1, p5);
    double rootLength = vRoot.Magnitude();

// these values are used to scale the refence Point to the absolut coordinate system for the cell
    double uScale, vScale;

// change the scale factors to the borders of a cell
// if a cell is defined

    gp_Pnt pC1, pC2, pC3, pC4;
    double LEXsi1, LEXsi2, TEXsi1, TEXsi2;
    double IBEta1, IBEta2, OBEta1, OBEta2;

    if (cell) {
    // get the Cornerpoints of the cell positions
    // take the eta / xsi values in the first step
        CCPACSWingCellPositionChordwise& PosLE = nCell->GetPositionLeadingEdge();
        CCPACSWingCellPositionChordwise& PosTE = nCell->GetPositionTrailingEdge();

        CCPACSWingCellPositionSpanwise& PosIB = nCell->GetPositionInnerBorder();
        CCPACSWingCellPositionSpanwise& PosOB = nCell->GetPositionOuterBorder();

        PosLE.getXsiInput(LEXsi1, LEXsi2);
        PosTE.getXsiInput(TEXsi1, TEXsi2);
        PosIB.getEtaDefinition(IBEta1, IBEta2);
        PosOB.getEtaDefinition(OBEta1, OBEta2);

        pC1 = wingStructureReference.GetMidplanePoint(IBEta1, LEXsi1);
        pC2 = wingStructureReference.GetMidplanePoint(OBEta1, LEXsi2);
        pC3 = wingStructureReference.GetMidplanePoint(IBEta2, TEXsi1);
        pC4 = wingStructureReference.GetMidplanePoint(OBEta2, TEXsi2);

        // set the border values for the cell

        gp_Vec vCellRoot(pC1,pC3);
        rootLength = vCellRoot.Magnitude();

        uScale = ((OBEta1 + OBEta2) / 2) - ((IBEta1 + IBEta2) / 2);
        vScale = ((TEXsi1 + TEXsi2) / 2) - ((LEXsi1 + LEXsi2) / 2);

        if (vScale < 0.) {
            vScale = (TEXsi1 - LEXsi1);
        }

    }
    else {
        pC1 = wingStructureReference.GetMidplanePoint(0., 0.);
        pC2 = wingStructureReference.GetMidplanePoint(0., 1.);
        pC3 = wingStructureReference.GetMidplanePoint(1., 0.);
        pC4 = wingStructureReference.GetMidplanePoint(1., 1.);
    }


// Step 4: calculate the reference point for the cell
// convert the reference point to the component segment eta / xsi 2D system

    if (cell) {

        double refEta = nCell->GetStringer().GetEta();
        double refXsi = nCell->GetStringer().GetXsi();

        pRef2D = gp_Pnt2d(IBEta1 + (refEta * uScale), (LEXsi1) + (refXsi * vScale));

        pitchScale = 1 - (refXsi * vScale);

    }

// Step 5: calculate "Number of stringers" or pitch
// depends on: which one is defined


    if ( (rootLength * pitchScale) < pitch ) {
        LOG(ERROR) << "Stringer construction error: Wrong value for stringer pitch. It is bigger than the available length." << std::endl;
        throw CTiglError("Stringer construction error: Wrong value for stringer pitch. It is bigger than the available length.");
    }

    if (nrStr == 0) {
        nrStr =  ((rootLength * pitchScale) / pitch);
    }
    else {
        pitch =  ((rootLength * pitchScale) / nrStr);
    }


// Step 6: Rotate the plane with the stringer angle around the z axis in the reference point
// The positive stringerangle (in CPCAS) is clockwise

    gp_Pnt pRef = wingStructureReference.GetMidplanePoint(pRef2D.X(), pRef2D.Y());

    gp_Vec angleReferencDir = GetAngleReference(angleRef);

    gp_Ax1 rotAxis(pRef, zRefDir);
    xRefDir = (zRefDir^angleReferencDir);
    xRefDir.Rotate(rotAxis, (stringerAngle));



// Step 7: Loop over the number of stringers

    for (int i = 0; i < nrStr; i++) {

    // create cutting plane

        gp_Ax3 refAx;

        if (cell) {
            refAx = gp_Ax3(pRef, xRefDir, gp_Vec(pC1, pC2));
        }
        else {
            refAx = gp_Ax3(pRef, xRefDir, yRefDirStern);
        }

        gp_Pln cutPlane(refAx);
    // create transformation vector
        gp_Vec vTrans = zRefDir^yRefDirStern;
        vTrans = -vTrans;
    // scale the translation vector
        vTrans.Scale(i * pitch);

        if (i != 0) {
            cutPlane.Translate(vTrans);
        }

    // convert the stringer cutting plane to a face
    // With the valus from the bounding Box
        TopoDS_Face cutFace = BRepLib_MakeFace(cutPlane, -bboxSize, bboxSize, -(zmax - zmin), (zmax - zmin)).Face();

        builder.Add(stringerCompound, TopoDS::Face(cutFace));

    }

    TopoDS_Shape planeShape = stringerCompound;

// Step 8: find the correct part of the loft
    // TODO: RENAME 
    TopoDS_Shape loftShape;
    if (side == UPPER_SIDE) {
        loftShape = wingStructureReference.GetUpperShape();
    }
    else {
        loftShape = wingStructureReference.GetLowerShape();
    }

// Step 10: find the right part of the loft if it is a cell
// create cutting planes for the cell Borders
// cut the lower or upper loft with the cell borders
// find the right face

    TopoDS_Shape result = CTiglCommon::cutShapes(loftShape, planeShape, true);

    TopTools_IndexedMapOfShape faceMap;
    if (cell) {
    // project the cornerpoints on the lower or upper surface
        TopoDS_Shape cutEdge1, cutEdge2, cutEdge3, cutEdge4;
        // build line along z reference axis for intersection points on loft
        gp_Vec offset(zRefDir*bboxSize);
		BRepBuilderAPI_MakeEdge edgeBuilder1(pC1.Translated(-offset), pC1.Translated(offset));
        cutEdge1 = edgeBuilder1.Edge();
        BRepBuilderAPI_MakeEdge edgeBuilder2(pC2.Translated(-offset), pC2.Translated(offset));
        cutEdge2 = edgeBuilder2.Edge();
        BRepBuilderAPI_MakeEdge edgeBuilder3(pC3.Translated(-offset), pC3.Translated(offset));
        cutEdge3 = edgeBuilder3.Edge();
        BRepBuilderAPI_MakeEdge edgeBuilder4(pC4.Translated(-offset), pC4.Translated(offset));
        cutEdge4 = edgeBuilder4.Edge();

        // find intersection points on loft geometry, use minimum distance for stability
        BRepExtrema_DistShapeShape ex1(loftShape, cutEdge1, Extrema_ExtFlag_MIN);
        ex1.Perform();
        pC1 = ex1.PointOnShape1(1);
        BRepExtrema_DistShapeShape ex2(loftShape, cutEdge2, Extrema_ExtFlag_MIN);
        ex2.Perform();
        pC2 = ex2.PointOnShape1(1);
        BRepExtrema_DistShapeShape ex3(loftShape, cutEdge3, Extrema_ExtFlag_MIN);
        ex3.Perform();
        pC3 = ex3.PointOnShape1(1);
        BRepExtrema_DistShapeShape ex4(loftShape, cutEdge4, Extrema_ExtFlag_MIN);
        ex4.Perform();
        pC4 = ex4.PointOnShape1(1);

    // check the 3d point coordinates

        if (pC1.X() > pC3.X()) {
            LOG(ERROR) << "Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe X values of the inner border." << std::endl;
            throw CTiglError("Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe X values of the inner border.");
        }
        if (pC2.X() > pC4.X()) {
            LOG(ERROR) << "Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe X values of the outer border." << std::endl;
            throw CTiglError("Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe X values of the outer border.");
        }
        if (pC1.Y() > pC2.Y()) {
            LOG(ERROR) << "Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe Y values on the leading edge." << std::endl;
            throw CTiglError("Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe Y values on the leading edge.");
        }
        if (pC3.Y() > pC4.Y()) {
            LOG(ERROR) << "Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe Y values on the trailing edge." << std::endl;
            throw CTiglError("Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe Y values on the trailing edge.");
        }

    // combine the cell cutting planes to a compound
        TopoDS_Compound Planecomp;
        builder.MakeCompound(Planecomp);

        gp_Pln cutPlaneLE, cutPlaneTE, cutPlaneIB, cutPlaneOB;

    // build the Leading edge cutting plane
        gp_Vec vCLE(pC1, pC2);
        vCLE.Normalize();
        gp_Pnt midPnt = CTiglCommon::getMidpoint(pC1, pC2);
        gp_Ax3 refAxLE(midPnt, -zRefDir^vCLE, vCLE);
        cutPlaneLE = gp_Pln(refAxLE);
        TopoDS_Shape planeShapeLE = BRepBuilderAPI_MakeFace(cutPlaneLE).Face();
    // build the Trailing edge cutting plane
        gp_Vec vCTE(pC3, pC4);
        vCTE.Normalize();
        midPnt = CTiglCommon::getMidpoint(pC3, pC4);
        gp_Ax3 refAxTE(midPnt, zRefDir^vCTE, vCTE);
        cutPlaneTE = gp_Pln(refAxTE);
        TopoDS_Shape planeShapeTE = BRepBuilderAPI_MakeFace(cutPlaneTE).Face();

        // build the inner border cutting plane
        gp_Vec vCIB(pC1, pC3);
        vCIB.Normalize();
        midPnt = CTiglCommon::getMidpoint(pC1, pC3);
        gp_Ax3 refAxIB(midPnt, zRefDir^vCIB, vCIB);
        cutPlaneIB = gp_Pln(refAxIB);
        TopoDS_Shape planeShapeIB = BRepBuilderAPI_MakeFace(cutPlaneIB).Face();

    // build the Outer border cutting plane
        gp_Vec vCOB(pC2, pC4);
        vCOB.Normalize();
        midPnt = CTiglCommon::getMidpoint(pC2, pC4);
        gp_Ax3 refAxOB(midPnt, -zRefDir^vCOB, vCOB);
        cutPlaneOB = gp_Pln(refAxOB);
        TopoDS_Shape planeShapeOB = BRepBuilderAPI_MakeFace(cutPlaneOB).Face();


    // If any border is defined by a rib or a spar, the cutting plane is changed

        CCPACSWingCellPositionChordwise& PosLE = nCell->GetPositionLeadingEdge();
        CCPACSWingCellPositionChordwise& PosTE = nCell->GetPositionTrailingEdge();

        CCPACSWingCellPositionSpanwise& PosIB = nCell->GetPositionInnerBorder();
        CCPACSWingCellPositionSpanwise& PosOB = nCell->GetPositionOuterBorder();

        double u05 = 0.5, u1 = 1., v0 = 0., v1 = 1.;

        if (PosLE.SparInput()) {
            planeShapeLE = PosLE.sparCuttingplane();
        }

        if (PosTE.SparInput()) {
            planeShapeTE = PosTE.sparCuttingplane();
        }

        if (PosIB.RibInput()) {
            planeShapeIB = PosIB.getRibCuttingPlane();
        }
        // if the inner border of the cell is the inner border of the Component segment
        // a cutting plane from the inner border of the WCS is created
        // this is necessary due to cutting precision
        else if (IBEta1 == 0. && IBEta2 == 0.) {
            BRepAdaptor_Surface surf(wingStructureReference.GetInnerFace());
            gp_Pnt p0 = surf.Value(u05, v0);
            gp_Pnt pU = surf.Value(u05, v1);
            gp_Pnt pV = surf.Value(u1, v0);
            gp_Ax3 ax0UV(p0, gp_Vec(p0, pU)^gp_Vec(p0, pV), gp_Vec(p0, pU));

            planeShapeIB = BRepBuilderAPI_MakeFace(gp_Pln(ax0UV)).Face();
        }

        if (PosOB.RibInput()) {
            planeShapeOB = PosOB.getRibCuttingPlane();
        }
        // if the outer border of the cell is the outer border of the Component segment
        // a cutting plane from the outer border of the WCS is created
        // this is necessary due to cutting precision
        else if (OBEta1 == 1. && OBEta2 == 1.) {
            BRepAdaptor_Surface surf(wingStructureReference.GetOuterFace());
            gp_Pnt p0 = surf.Value(u05, v0);
            gp_Pnt pU = surf.Value(u05, v1);
            gp_Pnt pV = surf.Value(u1, v0);
            gp_Ax3 ax0UV(p0, gp_Vec(p0, pU)^gp_Vec(p0, pV), gp_Vec(p0, pU));

            planeShapeOB = BRepBuilderAPI_MakeFace(gp_Pln(ax0UV)).Face();
        }


        // add the plane to the cutting compund
        builder.Add(Planecomp, planeShapeLE);
        builder.Add(Planecomp, planeShapeTE);
        builder.Add(Planecomp, planeShapeIB);
        builder.Add(Planecomp, planeShapeOB);

    // cut the lower or upper loft with the planes
        result = CTiglCommon::splitShape(result, Planecomp);

//      Debug Stuff
//         TopoDS_Compound compBla;
//         builder.MakeCompound(compBla);
//         builder.Add(compBla, BRepBuilderAPI_MakeFace(cutPlaneLE, -1e4, 1e4,-1e4,1e4).Face());
//         builder.Add(compBla, BRepBuilderAPI_MakeFace(cutPlaneTE, -1e4, 1e4,-1e4,1e4).Face());
//         builder.Add(compBla, BRepBuilderAPI_MakeFace(cutPlaneIB, -1e4, 1e4,-1e4,1e4).Face());
//         builder.Add(compBla, BRepBuilderAPI_MakeFace(cutPlaneOB, -1e4, 1e4,-1e4,1e4).Face());
//         TopoDS_Shape blaBla = compBla;
//         CTiglCommon::dumpShape(blaBla, "/caehome/mare102/Aerostruct_code" ,"bla1");
// 
//         CTiglCommon::dumpShape(planeShapeLE, "/caehome/mare102/Aerostruct_code" ,"bla2");
// 
        TopExp_Explorer ExpEdges;

    //find the right edges in the middle of the cell
        bool notFound = true;
        TopoDS_Compound cellLoftComp;
        builder.MakeCompound(cellLoftComp);
        bool sparTest = false, plainTest = false;

    // Loop over edges of the cutting operation with the loft and the cell border cutting planes
    // Check if the midpoint of the edges is in the direction of cutting plane normal
    // This means if it is inside the cell
    // all Cell border cutting plane normals show inside the cell
    // old old stringer construction before revision R455
    // old stringer construction before revision R577
        for (ExpEdges.Init(result, TopAbs_EDGE); ExpEdges.More(); ExpEdges.Next()) {

            BRepAdaptor_Curve curve(TopoDS::Edge(ExpEdges.Current()));
            double u_mn = curve.FirstParameter(), u_mx = curve.LastParameter();
            gp_Pnt pTest = curve.Value(u_mn + ((u_mx - u_mn) / 2));
//             gp_Pnt pStart = curve.Value(u_mn);
//             gp_Pnt pEnd = curve.Value(u_mx);

        // create each midpoint for the vector basis

            // test if the midplane point is behind the leading edge border plane
            // create an Ax1 from the Leading edge plane origin to the midpoint of the current face
            gp_Pnt midPnt = CTiglCommon::getMidpoint(pC1, pC2);
            gp_Vec vTest(midPnt, pTest);
            gp_Ax1 a1Test(midPnt, vTest);

            if (PosLE.SparInput()) {
                sparTest = SparSegmentsTest(cutPlaneLE.Axis(), pTest, planeShapeLE);
            }
            else {
                plainTest = a1Test.Angle(cutPlaneLE.Axis()) < (90.0 * (M_PI / 180.));
            }

            if (plainTest || sparTest) {
            // create an Ax1 from the trailing edge plane origin to the midpoint of the current face
                midPnt = CTiglCommon::getMidpoint(pC3, pC4);
                vTest = gp_Vec(midPnt, pTest);
                a1Test = gp_Ax1(midPnt, vTest);

                sparTest = false;
                plainTest = false;
                if (PosTE.SparInput()) {
                    sparTest = SparSegmentsTest(cutPlaneTE.Axis(), pTest, planeShapeTE);
                }
                else {
                    plainTest = a1Test.Angle(cutPlaneTE.Axis()) < (90.0 * (M_PI / 180.));
                }

                if (plainTest || sparTest) {

                // create an Ax1 from the inner border plane origin to the midpoint of the current face
                    midPnt = CTiglCommon::getMidpoint(pC1, pC3);
                    vTest = gp_Vec(midPnt, pTest);
                    a1Test = gp_Ax1(midPnt, vTest);
                    if (a1Test.Angle(cutPlaneIB.Axis()) < (90.0 * (M_PI / 180.))) {
                    // create an Ax1 from the outer border plane origin to the midpoint of the current face
                        midPnt = CTiglCommon::getMidpoint(pC2, pC4);
                        vTest = gp_Vec(midPnt, pTest);
                        a1Test = gp_Ax1(midPnt, vTest);
                        if (a1Test.Angle(cutPlaneOB.Axis()) < (90.0 * (M_PI / 180.))) {
                            builder.Add(currentCompound, TopoDS::Edge(ExpEdges.Current()));
                            notFound = false;
                        }
                    }
                }
            }

        }

        if (notFound) {
            LOG(ERROR) << "Wing stringer construction error: Can not find a matching edge for cell input CCPACSWingShell::BuildStringerGeometry!" << std::endl;
            throw CTiglError("Wing stringer construction error: Can not find a matching edge for cell input CCPACSWingShell::BuildStringerGeometry!");
        }


//  replace the Cutting planes by cutted cutting planes
        TopoDS_Shape cellCutFaces = CTiglCommon::splitShape(planeShape, Planecomp);

        faceMap.Clear();
        TopExp::MapShapes(cellCutFaces, TopAbs_FACE, faceMap);

    //find the right face in the middle of the cell
        notFound = true;
        builder.MakeCompound(stringerCompound);

    // Loop over faces of the cutting operation with the loft and the cell border cutting planes
    // Check if the midpoint of the faces is in the direction of cutting plane normal
    // This means if it is inside the cell
    // all Cell border cutting plane normals show inside the cell
    // old stringer construction before revision R455
        for (int f = 1; f <= faceMap.Extent(); f++) {
            BRepTools::UVBounds(TopoDS::Face(faceMap(f)), u_min, u_max, v_min, v_max);
            Handle(Geom_Surface) surf = BRep_Tool::Surface(TopoDS::Face(faceMap(f)));


            gp_Pnt pTest = surf->Value(u_min + ((u_max - u_min) / 2), v_min + ((v_max - v_min) / 2));;

            // test if the midplane point is behind the leading edge border plane
            // create an Ax1 from the Leading edge plane origin to the midpoint of the current face
        // create each midpoint for the vector basis
            gp_Pnt midPnt = CTiglCommon::getMidpoint(pC1, pC2);
            gp_Vec vTest(midPnt, pTest);
            gp_Ax1 a1Test(midPnt, vTest);


            if (PosLE.SparInput()) {
                sparTest = SparSegmentsTest(cutPlaneLE.Axis(), pTest, planeShapeLE);
            }
            else {
                plainTest = a1Test.Angle(cutPlaneLE.Axis()) < (90.0 * (M_PI / 180.));
            }

            if (plainTest || sparTest) {
            // create an Ax1 from the trailing edge plane origin to the midpoint of the current face
                midPnt = CTiglCommon::getMidpoint(pC3, pC4);
                vTest = gp_Vec(midPnt, pTest);
                a1Test = gp_Ax1(midPnt, vTest);

                sparTest = false;
                plainTest = false;
                if (PosTE.SparInput()) {
                    sparTest = SparSegmentsTest(cutPlaneTE.Axis(), pTest, planeShapeTE);
                }
                else {
                    plainTest = a1Test.Angle(cutPlaneTE.Axis()) < (90.0 * (M_PI / 180.));
                }

                if (plainTest || sparTest) {

                // create an Ax1 from the inner border plane origin to the midpoint of the current face
                    midPnt = CTiglCommon::getMidpoint(pC1, pC3);
                    vTest = gp_Vec(midPnt, pTest);
                    a1Test = gp_Ax1(midPnt, vTest);
                    if (a1Test.Angle(cutPlaneIB.Axis()) < (90.0 * (M_PI / 180.))) {
                    // create an Ax1 from the outer border plane origin to the midpoint of the current face
                        midPnt = CTiglCommon::getMidpoint(pC2, pC4);

                        vTest = gp_Vec(midPnt, pTest);
                        a1Test = gp_Ax1(midPnt, vTest);
                        if (a1Test.Angle(cutPlaneOB.Axis()) < (90.0 * (M_PI / 180.))) {
                            builder.Add(stringerCompound, TopoDS::Face(faceMap(f)));
                            notFound = false;
                        }
                    }
                }
            }
        }

        if (notFound) {
//             builder.MakeCompound(stringerCompound);
//             builder.Add(stringerCompound, planeShape);
            LOG(ERROR) << "Wing stringer construction error: Can not find a matching face for cell input during cut plane cut in CCPACSWingShell::BuildStringerGeometry!" << std::endl;
            throw CTiglError("Wing stringer construction error: Can not find a matching face for cell input during cut plane cut in CCPACSWingShell::BuildStringerGeometry!");
        }

    }


// add the constructructed Stringer geometry to the right Stringer class
    if (cell) {
        nCell->GetStringer().SetStringerGeometry(currentCompound);
        // add the resulting edges if a cell is defines
        builder.Add(compound, currentCompound);
    }
    else {
        stringer->SetStringerGeometry(result);
        // add the resulting edges if no cell is defines
        builder.Add(compound,result);
    }

// replace the TopoDS_Shape by the constructed (and preveously existing) geometry
    stringerEdges = compound;
// set cutting faces for cutting segment mit stringers

    builder.Add(CCuttingCompound, stringerCompound);

    mCuttingPlanes = CCuttingCompound;
}

// [[CAS_AES]] added method
void CCPACSWingShell::BuildExplicitStringerGeometry(CCPACSWingCell* nCell) 
{

// Variable declaration
    TopoDS_Compound compound;
    TopoDS_Compound CCuttingCompound;
    TopoDS_Compound stringerCompound;
    TopoDS_Compound currentCompound;
    BRep_Builder builder;

// variable for uv bounds
    double u_min=0., u_max=0., v_min=0., v_max=0.;

        // if a cell is defined with ribs and spars, the eta xsi values are calculated
    nCell->CalcEtaXsi();

    for (int s = 1; s <= nCell->NumberOfExplicitStringer(); s++) {

        std::vector<gp_Pnt> startPoints = std::vector<gp_Pnt>();
        std::vector<gp_Pnt> endPoints = std::vector<gp_Pnt>();

    // define stringer construction variables
        int nrStr = 1;
        double stringerAngle = 0.;
        std::string angleRef;
        double xsiIB1 = -1., xsiIB2 = -1., xsiOB1 = -1., xsiOB2 = -1.;

        CCPACSExplicitWingStringer* explicitStringer = nCell->GetExplicitStringerbyIndex(s);

    // initialise compound
        builder.MakeCompound(compound);
        builder.MakeCompound(CCuttingCompound);
        builder.MakeCompound(stringerCompound);
        builder.MakeCompound(currentCompound);
    
    // There can be another cell on the shell
        if (!stringerEdges.IsNull())
            builder.Add(compound, stringerEdges);
        if (!stringerEdges.IsNull())
            builder.Add(CCuttingCompound, mCuttingPlanes);
    
        // determine diagonal vector of loft bounding box (e.g. used for size of cut faces)
        Bnd_Box boundingBox;
        BRepBndLib::Add(wingStructureReference.GetUpperShape(), boundingBox);
        BRepBndLib::Add(wingStructureReference.GetLowerShape(), boundingBox);
        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
        boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
        gp_Vec diagonal(xmax-xmin, ymax-ymin, zmax-zmin);
        Standard_Real bboxSize = diagonal.Magnitude();
    
    // Step 1 : get the stringer definition from the cell ...
        
    
        nrStr = explicitStringer->GetNumOfStringers();
        stringerAngle = -(explicitStringer->GetAngle()) * (M_PI / 180);
        angleRef = explicitStringer->GetAngleReference();

        xsiIB1 = explicitStringer->GetXsiIB1();
        xsiIB2 = explicitStringer->GetXsiIB2();
        xsiOB1 = explicitStringer->GetXsiOB1();
        xsiOB2 = explicitStringer->GetXsiOB2();
    
    
    // Step 2:  Get the x and z reference direction for the stringer cutting planes
    
        gp_Pnt p1 = wingStructureReference.GetLeadingEdgePoint(0);
        gp_Pnt p2 = wingStructureReference.GetLeadingEdgePoint(1);
        gp_Vec yRefDir(p1,p2);
        yRefDir.Normalize();
    
        // create a reference direction without sweep angle
        gp_Pnt p2stern = gp_Pnt(p1.X(), p2.Y(), p2.Z());
        gp_Vec yRefDirStern(p1,p2stern);
        yRefDirStern.Normalize();

    
        CCPACSWing& CW = wingStructureReference.GetWing();
        gp_Pnt p3(0.,0.,0.);
        gp_Pnt p4(0.,0.,1.);
        gp_Pnt p7(0.,1.,0.);
    
        gp_Vec globalYDir(p3,p7);
        globalYDir.Normalize();
    
    //     Step 3: creation of a Component segment midplane face
    // It is necessary to seperate between the upper and the lower surface
        gp_Pnt p5 = wingStructureReference.GetTrailingEdgePoint(0);
        gp_Pnt p6 = wingStructureReference.GetTrailingEdgePoint(1);
    
        gp_Vec xRefDir(p1,p5);
        xRefDir.Normalize();
    
        gp_Vec zRefDir(-yRefDirStern^xRefDir);
    //     gp_Vec zRefDir(-globalYDir^xRefDir);
        zRefDir.Normalize();
    
    // change the scale factors to the borders of a cell
    // if a cell is defined
    

    // Step 4: find the correct part of the loft
        // TODO: RENAME 
        TopoDS_Shape loftShape;
        if (side == UPPER_SIDE) {
            loftShape = wingStructureReference.GetUpperShape();
        }
        else {
            loftShape = wingStructureReference.GetLowerShape();
        }
    

        gp_Pnt pC1, pC2, pC3, pC4;
        double LEXsi1, LEXsi2, TEXsi1, TEXsi2;
        double IBEta1, IBEta2, OBEta1, OBEta2;
    
    // get the Cornerpoints of the cell positions
    // take the eta / xsi values in the first step
        CCPACSWingCellPositionChordwise& PosLE = nCell->GetPositionLeadingEdge();
        CCPACSWingCellPositionChordwise& PosTE = nCell->GetPositionTrailingEdge();
    
        CCPACSWingCellPositionSpanwise& PosIB = nCell->GetPositionInnerBorder();
        CCPACSWingCellPositionSpanwise& PosOB = nCell->GetPositionOuterBorder();
    
        PosLE.getXsiInput(LEXsi1, LEXsi2);
        PosTE.getXsiInput(TEXsi1, TEXsi2);
        PosIB.getEtaDefinition(IBEta1, IBEta2);
        PosOB.getEtaDefinition(OBEta1, OBEta2);
    
        pC1 = wingStructureReference.GetMidplanePoint(IBEta1, LEXsi1);
        pC2 = wingStructureReference.GetMidplanePoint(OBEta1, LEXsi2);
        pC3 = wingStructureReference.GetMidplanePoint(IBEta2, TEXsi1);
        pC4 = wingStructureReference.GetMidplanePoint(OBEta2, TEXsi2);
        // project the cornerpoints on the lower or upper surface
        TopoDS_Shape cutEdge1, cutEdge2, cutEdge3, cutEdge4;
        // build line along z reference axis for intersection points on loft
        gp_Vec offset(zRefDir*bboxSize);
        BRepBuilderAPI_MakeEdge edgeBuilder1(pC1.Translated(-offset), pC1.Translated(offset));
        cutEdge1 = edgeBuilder1.Edge();
        BRepBuilderAPI_MakeEdge edgeBuilder2(pC2.Translated(-offset), pC2.Translated(offset));
        cutEdge2 = edgeBuilder2.Edge();
        BRepBuilderAPI_MakeEdge edgeBuilder3(pC3.Translated(-offset), pC3.Translated(offset));
        cutEdge3 = edgeBuilder3.Edge();
        BRepBuilderAPI_MakeEdge edgeBuilder4(pC4.Translated(-offset), pC4.Translated(offset));
        cutEdge4 = edgeBuilder4.Edge();
    
        // find intersection points on loft geometry, use minimum distance for stability
        BRepExtrema_DistShapeShape ex1(loftShape, cutEdge1, Extrema_ExtFlag_MIN);
        ex1.Perform();
        pC1 = ex1.PointOnShape1(1);
        BRepExtrema_DistShapeShape ex2(loftShape, cutEdge2, Extrema_ExtFlag_MIN);
        ex2.Perform();
        pC2 = ex2.PointOnShape1(1);
        BRepExtrema_DistShapeShape ex3(loftShape, cutEdge3, Extrema_ExtFlag_MIN);
        ex3.Perform();
        pC3 = ex3.PointOnShape1(1);
        BRepExtrema_DistShapeShape ex4(loftShape, cutEdge4, Extrema_ExtFlag_MIN);
        ex4.Perform();
        pC4 = ex4.PointOnShape1(1);

    // check the 3d point coordinates
    
        if (pC1.X() > pC3.X()) {
            LOG(ERROR) << "Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe X values of the inner border." << std::endl;
            throw CTiglError("Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe X values of the inner border.");
        }
        if (pC2.X() > pC4.X()) {
            LOG(ERROR) << "Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe X values of the outer border." << std::endl;
            throw CTiglError("Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe X values of the outer border.");
        }
        if (pC1.Y() > pC2.Y()) {
            LOG(ERROR) << "Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe Y values on the leading edge." << std::endl;
            throw CTiglError("Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe Y values on the leading edge.");
        }
        if (pC3.Y() > pC4.Y()) {
            LOG(ERROR) << "Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe Y values on the trailing edge." << std::endl;
            throw CTiglError("Wing stringer construction error: Wrong value for cell input in CCPACSWingShell::BuildStringerGeometry!\nThe Y values on the trailing edge.");
        }

    // 
    //     // set the border values for the cell
    // 
        gp_Vec vCellRoot(pC1,pC3);
        double rootLength = vCellRoot.Magnitude();

    // Step 5: Genertate the starting Points of the stringer

        if ((explicitStringer->GetDefintionType() == CCPACSExplicitWingStringer::START_END) || 
            (explicitStringer->GetDefintionType() == CCPACSExplicitWingStringer::START_ANGLE)) {
            BRepAdaptor_Curve curve(BRepBuilderAPI_MakeEdge(pC1, pC3));
            double u_mn = curve.FirstParameter(), u_mx = curve.LastParameter();

            if (nrStr == 1) {
                gp_Pnt pTemp = curve.Value(u_mn + xsiIB1 * (u_mx - u_mn));
                startPoints.push_back(pTemp);
            }
            else if (nrStr > 1) {
                for (int k = 0; k < nrStr; k++) {
                    double c = xsiIB1 + k * ((xsiIB2 - xsiIB1) / (nrStr - 1));
                    gp_Pnt pTemp = curve.Value(u_mn + c * (u_mx - u_mn));
                    startPoints.push_back(pTemp);
                }

            }
            else {
                LOG(ERROR) << "Error: Wrong value for number of stringer in BuildExplicitStringerGeometry." << std::endl;
                throw CTiglError("Error: Wrong value for number of stringer in BuildExplicitStringerGeometry.");
            }
        }

    // Step 6: Genertate the ending Points of the stringer

        if ((explicitStringer->GetDefintionType() == CCPACSExplicitWingStringer::START_END) || 
            (explicitStringer->GetDefintionType() == CCPACSExplicitWingStringer::END_ANGLE)) {
            BRepAdaptor_Curve curve(BRepBuilderAPI_MakeEdge(pC2, pC4));
            double u_mn = curve.FirstParameter(), u_mx = curve.LastParameter();

            if (nrStr == 1) {
                gp_Pnt pTemp = curve.Value(u_mn + xsiOB1 * (u_mx - u_mn));
                endPoints.push_back(pTemp);
            }
            else if (nrStr > 1) {
                for (int k = 0; k <= nrStr; k++) {
                    double c = xsiOB1 + k * ((xsiOB2 - xsiOB1) / (nrStr - 1));
                    gp_Pnt pTemp = curve.Value(u_mn + c * (u_mx - u_mn));
                    endPoints.push_back(pTemp);
                }

            }
            else {
                LOG(ERROR) << "Error: Wrong value for number of stringer in BuildExplicitStringerGeometry." << std::endl;
                throw CTiglError("Error: Wrong value for number of stringer in BuildExplicitStringerGeometry.");
            }
        }

    // Step 7: Generate Stringer planes for  case START_END

        if (explicitStringer->GetDefintionType() == CCPACSExplicitWingStringer::START_END) {
            for (int i = 0; i < startPoints.size(); i++) {
                xRefDir = (zRefDir^gp_Vec(startPoints[i], endPoints[i]));
                gp_Ax3 refAx(startPoints[i], xRefDir, gp_Vec(startPoints[i], endPoints[i]));
                gp_Pln cutPlane(refAx);

                TopoDS_Face cutFace = BRepLib_MakeFace(cutPlane, -bboxSize, bboxSize, -(zmax - zmin), (zmax - zmin)).Face();
                builder.Add(stringerCompound, TopoDS::Face(cutFace));
            }
        }

    // Step 8: Generate Stringer planes for  case START_ANGLE

        if (explicitStringer->GetDefintionType() == CCPACSExplicitWingStringer::START_ANGLE) {
            gp_Vec angleReferencDir = GetAngleReference(angleRef);

            for (int i = 0; i < startPoints.size(); i++) {
                xRefDir = (zRefDir^angleReferencDir);

                gp_Ax1 rotAxis(startPoints[i], zRefDir);
                xRefDir.Rotate(rotAxis, stringerAngle);

                gp_Ax3 refAx(startPoints[i], xRefDir, gp_Vec(pC1, pC2));
                gp_Pln cutPlane(refAx);

                TopoDS_Face cutFace = BRepLib_MakeFace(cutPlane, -bboxSize, bboxSize, -(zmax - zmin), (zmax - zmin)).Face();

                builder.Add(stringerCompound, TopoDS::Face(cutFace));
            }
        }

    // Step 9: Generate Stringer planes for  case START_ANGLE

        if (explicitStringer->GetDefintionType() == CCPACSExplicitWingStringer::END_ANGLE) {
            gp_Vec angleReferencDir = GetAngleReference(angleRef);
            xRefDir = (zRefDir^angleReferencDir);

            for (int i = 0; i < endPoints.size(); i++) {
                xRefDir = (zRefDir^angleReferencDir);

                gp_Ax1 rotAxis(endPoints[i], zRefDir);
                xRefDir.Rotate(rotAxis, stringerAngle);

                gp_Ax3 refAx(endPoints[i], xRefDir, gp_Vec(pC1, pC2));
                gp_Pln cutPlane(refAx);

                TopoDS_Face cutFace = BRepLib_MakeFace(cutPlane, -bboxSize, bboxSize, -(zmax - zmin), (zmax - zmin)).Face();

                builder.Add(stringerCompound, TopoDS::Face(cutFace));
            }
        }


        // stringer cutPlanes
        TopoDS_Shape planeShape = stringerCompound;
    
    // Step 10: find the right part of the loft if it is a cell
    // create cutting planes for the cell Borders
    // cut the lower or upper loft with the cell borders
    // find the right face
    
        TopoDS_Shape result = CTiglCommon::cutShapes(loftShape, planeShape, true);
    
        TopTools_IndexedMapOfShape faceMap;
        
    // combine the cell cutting planes to a compound
        TopoDS_Compound Planecomp;
        builder.MakeCompound(Planecomp);
    
        gp_Pln cutPlaneLE, cutPlaneTE, cutPlaneIB, cutPlaneOB;
    
    // build the Leading edge cutting plane
        gp_Vec vCLE(pC1, pC2);
        vCLE.Normalize();
        gp_Pnt midPnt = CTiglCommon::getMidpoint(pC1, pC2);
        gp_Ax3 refAxLE(midPnt, -zRefDir^vCLE, vCLE);
        cutPlaneLE = gp_Pln(refAxLE);
        TopoDS_Shape planeShapeLE = BRepBuilderAPI_MakeFace(cutPlaneLE).Face();
    // build the Trailing edge cutting plane
        gp_Vec vCTE(pC3, pC4);
        vCTE.Normalize();
        midPnt = CTiglCommon::getMidpoint(pC3, pC4);
        gp_Ax3 refAxTE(midPnt, zRefDir^vCTE, vCTE);
        cutPlaneTE = gp_Pln(refAxTE);
        TopoDS_Shape planeShapeTE = BRepBuilderAPI_MakeFace(cutPlaneTE).Face();
    
        // build the inner border cutting plane
        gp_Vec vCIB(pC1, pC3);
        vCIB.Normalize();
        midPnt = CTiglCommon::getMidpoint(pC1, pC3);
        gp_Ax3 refAxIB(midPnt, zRefDir^vCIB, vCIB);
        cutPlaneIB = gp_Pln(refAxIB);
        TopoDS_Shape planeShapeIB = BRepBuilderAPI_MakeFace(cutPlaneIB).Face();
    
    // build the Outer border cutting plane
        gp_Vec vCOB(pC2, pC4);
        vCOB.Normalize();
        midPnt = CTiglCommon::getMidpoint(pC2, pC4);
        gp_Ax3 refAxOB(midPnt, -zRefDir^vCOB, vCOB);
        cutPlaneOB = gp_Pln(refAxOB);
        TopoDS_Shape planeShapeOB = BRepBuilderAPI_MakeFace(cutPlaneOB).Face();

        double u05 = 0.5, u1 = 1., v0 = 0., v1 = 1.;
    
        if (PosLE.SparInput()) {
            planeShapeLE = PosLE.sparCuttingplane();
        }
    
        if (PosTE.SparInput()) {
            planeShapeTE = PosTE.sparCuttingplane();
        }
    
        if (PosIB.RibInput()) {
            planeShapeIB = PosIB.getRibCuttingPlane();
        }
        // if the inner border of the cell is the inner border of the Component segment
        // a cutting plane from the inner border of the WCS is created
        // this is necessary due to cutting precision
        else if (IBEta1 == 0. && IBEta2 == 0.) {
            BRepAdaptor_Surface surf(wingStructureReference.GetInnerFace());
            gp_Pnt p0 = surf.Value(u05, v0);
            gp_Pnt pU = surf.Value(u05, v1);
            gp_Pnt pV = surf.Value(u1, v0);
            gp_Ax3 ax0UV(p0, gp_Vec(p0, pU)^gp_Vec(p0, pV), gp_Vec(p0, pU));
    
            planeShapeIB = BRepBuilderAPI_MakeFace(gp_Pln(ax0UV)).Face();
        }
    
        if (PosOB.RibInput()) {
            planeShapeOB = PosOB.getRibCuttingPlane();
        }
        // if the outer border of the cell is the outer border of the Component segment
        // a cutting plane from the outer border of the WCS is created
        // this is necessary due to cutting precision
        else if (OBEta1 == 1. && OBEta2 == 1.) {
            BRepAdaptor_Surface surf(wingStructureReference.GetOuterFace());
            gp_Pnt p0 = surf.Value(u05, v0);
            gp_Pnt pU = surf.Value(u05, v1);
            gp_Pnt pV = surf.Value(u1, v0);
            gp_Ax3 ax0UV(p0, gp_Vec(p0, pU)^gp_Vec(p0, pV), gp_Vec(p0, pU));
    
            planeShapeOB = BRepBuilderAPI_MakeFace(gp_Pln(ax0UV)).Face();
        }
    
    
        // add the plane to the cutting compund
        builder.Add(Planecomp, planeShapeLE);
        builder.Add(Planecomp, planeShapeTE);
        builder.Add(Planecomp, planeShapeIB);
        builder.Add(Planecomp, planeShapeOB);
    
    // cut the lower or upper loft with the planes
        result = CTiglCommon::splitShape(result, Planecomp);
    
    //      Debug Stuff
    //         TopoDS_Compound compBla;
    //         builder.MakeCompound(compBla);
    //         builder.Add(compBla, BRepBuilderAPI_MakeFace(cutPlaneLE, -1e4, 1e4,-1e4,1e4).Face());
    //         builder.Add(compBla, BRepBuilderAPI_MakeFace(cutPlaneTE, -1e4, 1e4,-1e4,1e4).Face());
    //         builder.Add(compBla, BRepBuilderAPI_MakeFace(cutPlaneIB, -1e4, 1e4,-1e4,1e4).Face());
    //         builder.Add(compBla, BRepBuilderAPI_MakeFace(cutPlaneOB, -1e4, 1e4,-1e4,1e4).Face());
    //         TopoDS_Shape blaBla = compBla;
    //         CTiglCommon::dumpShape(blaBla, "/caehome/mare102/Aerostruct_code" ,"bla1");
    // 
    //         CTiglCommon::dumpShape(planeShapeLE, "/caehome/mare102/Aerostruct_code" ,"bla2");
    // 
        TopExp_Explorer ExpEdges;
    
    //find the right edges in the middle of the cell
        bool notFound = true;
        TopoDS_Compound cellLoftComp;
        builder.MakeCompound(cellLoftComp);
        bool sparTest = false, plainTest = false;
    
    // Loop over edges of the cutting operation with the loft and the cell border cutting planes
    // Check if the midpoint of the edges is in the direction of cutting plane normal
    // This means if it is inside the cell
    // all Cell border cutting plane normals show inside the cell
    // old old stringer construction before revision R455
    // old stringer construction before revision R577
        for (ExpEdges.Init(result, TopAbs_EDGE); ExpEdges.More(); ExpEdges.Next()) {
    
            BRepAdaptor_Curve curve(TopoDS::Edge(ExpEdges.Current()));
            double u_mn = curve.FirstParameter(), u_mx = curve.LastParameter();
            gp_Pnt pTest = curve.Value(u_mn + ((u_mx - u_mn) / 2));
    //             gp_Pnt pStart = curve.Value(u_mn);
    //             gp_Pnt pEnd = curve.Value(u_mx);
    
        // create each midpoint for the vector basis
    
            // test if the midplane point is behind the leading edge border plane
            // create an Ax1 from the Leading edge plane origin to the midpoint of the current face
            gp_Pnt midPnt = CTiglCommon::getMidpoint(pC1, pC2);
            gp_Vec vTest(midPnt, pTest);
            gp_Ax1 a1Test(midPnt, vTest);
    
            if (PosLE.SparInput()) {
                sparTest = SparSegmentsTest(cutPlaneLE.Axis(), pTest, planeShapeLE);
            }
            else {
                plainTest = a1Test.Angle(cutPlaneLE.Axis()) < (90.0 * (M_PI / 180.));
            }
    
            if (plainTest || sparTest) {
            // create an Ax1 from the trailing edge plane origin to the midpoint of the current face
                midPnt = CTiglCommon::getMidpoint(pC3, pC4);
                vTest = gp_Vec(midPnt, pTest);
                a1Test = gp_Ax1(midPnt, vTest);
    
                sparTest = false;
                plainTest = false;
                if (PosTE.SparInput()) {
                    sparTest = SparSegmentsTest(cutPlaneTE.Axis(), pTest, planeShapeTE);
                }
                else {
                    plainTest = a1Test.Angle(cutPlaneTE.Axis()) < (90.0 * (M_PI / 180.));
                }
    
                if (plainTest || sparTest) {
    
                // create an Ax1 from the inner border plane origin to the midpoint of the current face
                    midPnt = CTiglCommon::getMidpoint(pC1, pC3);
                    vTest = gp_Vec(midPnt, pTest);
                    a1Test = gp_Ax1(midPnt, vTest);
                    if (a1Test.Angle(cutPlaneIB.Axis()) < (90.0 * (M_PI / 180.))) {
                    // create an Ax1 from the outer border plane origin to the midpoint of the current face
                        midPnt = CTiglCommon::getMidpoint(pC2, pC4);
                        vTest = gp_Vec(midPnt, pTest);
                        a1Test = gp_Ax1(midPnt, vTest);
                        if (a1Test.Angle(cutPlaneOB.Axis()) < (90.0 * (M_PI / 180.))) {
                            builder.Add(currentCompound, TopoDS::Edge(ExpEdges.Current()));
                            notFound = false;
                        }
                    }
                }
            }
    
        }
    
        if (notFound) {
            LOG(ERROR) << "Wing stringer construction error: Can not find a matching edge for cell input CCPACSWingShell::BuildExplicitStringerGeometry!" << std::endl;
            throw CTiglError("Wing stringer construction error: Can not find a matching edge for cell input CCPACSWingShell::BuildExplicitStringerGeometry!");
        }
    
    
    //  replace the Cutting planes by cutted cutting planes
        TopoDS_Shape cellCutFaces = CTiglCommon::splitShape(planeShape, Planecomp);
    
        faceMap.Clear();
        TopExp::MapShapes(cellCutFaces, TopAbs_FACE, faceMap);
    
    //find the right face in the middle of the cell
        notFound = true;
        builder.MakeCompound(stringerCompound);
    
        // Loop over faces of the cutting operation with the loft and the cell border cutting planes
        // Check if the midpoint of the faces is in the direction of cutting plane normal
        // This means if it is inside the cell
        // all Cell border cutting plane normals show inside the cell
        // old stringer construction before revision R455
        for (int f = 1; f <= faceMap.Extent(); f++) {
            BRepTools::UVBounds(TopoDS::Face(faceMap(f)), u_min, u_max, v_min, v_max);
            Handle(Geom_Surface) surf = BRep_Tool::Surface(TopoDS::Face(faceMap(f)));
    
    
            gp_Pnt pTest = surf->Value(u_min + ((u_max - u_min) / 2), v_min + ((v_max - v_min) / 2));;
    
            // test if the midplane point is behind the leading edge border plane
            // create an Ax1 from the Leading edge plane origin to the midpoint of the current face
        // create each midpoint for the vector basis
            gp_Pnt midPnt = CTiglCommon::getMidpoint(pC1, pC2);
            gp_Vec vTest(midPnt, pTest);
            gp_Ax1 a1Test(midPnt, vTest);
    
    
            if (PosLE.SparInput()) {
                sparTest = SparSegmentsTest(cutPlaneLE.Axis(), pTest, planeShapeLE);
            }
            else {
                plainTest = a1Test.Angle(cutPlaneLE.Axis()) < (90.0 * (M_PI / 180.));
            }
    
            if (plainTest || sparTest) {
            // create an Ax1 from the trailing edge plane origin to the midpoint of the current face
                midPnt = CTiglCommon::getMidpoint(pC3, pC4);
                vTest = gp_Vec(midPnt, pTest);
                a1Test = gp_Ax1(midPnt, vTest);
    
                sparTest = false;
                plainTest = false;
                if (PosTE.SparInput()) {
                    sparTest = SparSegmentsTest(cutPlaneTE.Axis(), pTest, planeShapeTE);
                }
                else {
                    plainTest = a1Test.Angle(cutPlaneTE.Axis()) < (90.0 * (M_PI / 180.));
                }
    
                if (plainTest || sparTest) {
    
                // create an Ax1 from the inner border plane origin to the midpoint of the current face
                    midPnt = CTiglCommon::getMidpoint(pC1, pC3);
                    vTest = gp_Vec(midPnt, pTest);
                    a1Test = gp_Ax1(midPnt, vTest);
                    if (a1Test.Angle(cutPlaneIB.Axis()) < (90.0 * (M_PI / 180.))) {
                    // create an Ax1 from the outer border plane origin to the midpoint of the current face
                        midPnt = CTiglCommon::getMidpoint(pC2, pC4);
    
                        vTest = gp_Vec(midPnt, pTest);
                        a1Test = gp_Ax1(midPnt, vTest);
                        if (a1Test.Angle(cutPlaneOB.Axis()) < (90.0 * (M_PI / 180.))) {
                            builder.Add(stringerCompound, TopoDS::Face(faceMap(f)));
                            notFound = false;
                        }
                    }
                }
            }
        }
    
        if (notFound) {
    //             builder.MakeCompound(stringerCompound);
    //             builder.Add(stringerCompound, planeShape);
            LOG(ERROR) << "Wing stringer construction error: Can not find a matching face for cell input during cut plane cut in CCPACSWingShell::BuildExplicitStringerGeometry!" << std::endl;
            throw CTiglError("Wing stringer construction error: Can not find a matching face for cell input during cut plane cut in CCPACSWingShell::BuildExplicitStringerGeometry!");
        }

        explicitStringer->SetStringerGeometry(currentCompound);

        // add the resulting edges if a cell is defines
        builder.Add(compound, currentCompound);
    
    // replace the TopoDS_Shape by the constructed (and preveously existing) geometry
        stringerEdges = compound;
    // set geometry valid
        isvalid = true;
    // set cutting faces for cutting segment mit stringers
    
        builder.Add(CCuttingCompound, stringerCompound);
    
        mCuttingPlanes = CCuttingCompound;
    }
}

// [[CAS_AES]] added method
bool CCPACSWingShell::SparSegmentsTest(gp_Ax1 nNormal, gp_Pnt nTestPoint, TopoDS_Shape nSparSegments, bool doWingTrafo)
{
    BRep_Builder builder;
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(nSparSegments, TopAbs_FACE, faceMap);
    double u_min=0., u_max=0., v_min=0., v_max=0.;
    
    // for symmetry
    nTestPoint.SetY(fabs(nTestPoint.Y()));

    for (int f = 1; f <= faceMap.Extent(); f++) {
        TopoDS_Face loftFace = TopoDS::Face(faceMap(f));
        BRepAdaptor_Surface surf(loftFace);
        Handle(Geom_Surface) geomSurf = BRep_Tool::Surface(loftFace);


        BRepTools::UVBounds(TopoDS::Face(faceMap(f)), u_min, u_max, v_min, v_max);
        gp_Pnt midPnt = surf.Value(u_min + ((u_max - u_min) / 2), v_min + ((v_max - v_min) / 2));


        gp_Pnt startPnt = surf.Value(u_min, v_min + ((v_max - v_min) / 2));
        gp_Pnt endPnt = surf.Value(u_max, v_min + ((v_max - v_min) / 2));
        
        if (doWingTrafo) {
            startPnt = wingStructureReference.GetWing().GetWingTransformation().Transform(startPnt);
            midPnt = wingStructureReference.GetWing().GetWingTransformation().Transform(midPnt);
            endPnt = wingStructureReference.GetWing().GetWingTransformation().Transform(endPnt);
        }

    // if the U / V direction of the spar plane is changed

        gp_Ax1 a1Test0 = gp_Ax1(startPnt, gp_Vec(startPnt, endPnt));
        gp_Ax1 a1TestZ = gp_Ax1(startPnt, gp_Vec(gp_Pnt(0.,0.,0.), gp_Pnt(0.,0.,1.)));
        gp_Ax1 a1Test_mZ = gp_Ax1(startPnt, gp_Vec(gp_Pnt(0.,0.,0.), gp_Pnt(0.,0.,-1.)));


        if (a1Test0.Angle(a1TestZ) < (20.0 * (M_PI / 180.))) {
            startPnt = surf.Value(u_min + ((u_max - u_min) / 2), v_min);
            endPnt = surf.Value(u_min + ((u_max - u_min) / 2), v_max);
        }
        else if (a1Test0.Angle(a1Test_mZ) < (20.0 * (M_PI / 180.))) {
            startPnt = surf.Value(u_min + ((u_max - u_min) / 2), v_min);
            endPnt = surf.Value(u_min + ((u_max - u_min) / 2), v_max);
        }

        // Here it is checked if the stringer is in the Y area of the corresponding spar face

        if (endPnt.Y() > startPnt.Y()) {
            if (startPnt.Y() > nTestPoint.Y() || endPnt.Y() < nTestPoint.Y()) {
                continue;
            }
        }
        else {
            if (startPnt.Y() < fabs(nTestPoint.Y()) || endPnt.Y() > fabs(nTestPoint.Y())) {
                continue;
            }
        }


        // project test point onto the surface
        Handle(ShapeAnalysis_Surface) SA_surf = new ShapeAnalysis_Surface(geomSurf);
        gp_Pnt2d uv = SA_surf->ValueOfUV(nTestPoint, 0.0);
        gp_Pnt pTestProj = surf.Value(uv.X(), uv.Y());
        
        
        gp_Pln facePlane = surf.Plane();
        gp_Ax1 planeNormal = facePlane.Axis();
        planeNormal.SetLocation(pTestProj);

        if (nNormal.Angle(planeNormal) > (90.0 * (M_PI / 180.))) {
            planeNormal = planeNormal.Reversed();
        }
        
        
        gp_Vec vTest = gp_Vec(pTestProj, nTestPoint);
        
        if (vTest.Magnitude() == 0.) {
            continue;
        }
        
        gp_Ax1 a1Test = gp_Ax1(pTestProj, vTest);

        if (a1Test.Angle(planeNormal) < (89.0 * (M_PI / 180.))) {
            return true;
        }
        else {
            continue;
        }

    }

    return false;
}

gp_Vec CCPACSWingShell::GetAngleReference(std::string nAngelRef)
{

    if (nAngelRef.compare("leadingEdge") == 0) {
        return gp_Vec(wingStructureReference.GetMidplanePoint(0., 0.), wingStructureReference.GetMidplanePoint(1., 0.));
    }
    else if (nAngelRef.compare("trailingEdge") == 0) {
        return gp_Vec(wingStructureReference.GetMidplanePoint(0., 1.), wingStructureReference.GetMidplanePoint(1., 1.));
    }
    else if (nAngelRef.compare("") == 0 || nAngelRef.compare("globalY") == 0) {
        return gp_Vec(gp_Pnt(0.,0.,0.), gp_Pnt(0.,1.,0.));
    }

    try {
        CCPACSWingSpars& WSpars = wingStructureReference.GetStructure().GetSpars();
        CCPACSWingSparSegments& WSSegments = WSpars.GetSparSegments();
        CCPACSWingSparSegment& WSSegment = WSSegments.GetSparSegment(nAngelRef);

        CCPACSWingSparPositionUIDs* CWSPUids = WSSegment.getSparPositionUIDs();

        std::string innerPosUId = CWSPUids->GetSparPositionUID(1);
        std::string outerPosUId = CWSPUids->GetSparPositionUID(CWSPUids->GetSparPositionUIDCount());

        CCPACSWingSparPosition& innerPos = WSpars.GetSparPosition(innerPosUId);
        CCPACSWingSparPosition& outerPos = WSpars.GetSparPosition(outerPosUId);

        gp_Pnt innerPnt = wingStructureReference.GetMidplanePoint(innerPos.GetEta(), innerPos.GetXsi());
        gp_Pnt outerPnt = wingStructureReference.GetMidplanePoint(outerPos.GetEta(), outerPos.GetXsi());

        return gp_Vec(innerPnt, outerPnt);
    }
    catch (...) {
        LOG(ERROR) << "Wing stringer construction error: The angle reference is not correct." << std::endl;
        throw CTiglError("Wing stringer construction error: The angle reference is not correct.");
    }


}

} // namespace tigl
