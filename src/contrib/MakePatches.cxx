/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* This file was created by OpenCASCADE for the German Aerospace Center.
* It will reside in TiGL, until this Algorithm becomes part of
* OpenCASCADE.
*
* For now, the file will be licensed under the Apache License, Version 2.0.
*    http://www.apache.org/licenses/LICENSE-2.0
*/

#include "tigl_config.h"
#include "tiglcommonfunctions.h"

#include <MakePatches.hxx>
#include <MakeLoops.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_HArray2OfShape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Edge.hxx>
#include <Geom_Curve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>
#include <TColGeom_SequenceOfBoundedCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BoundedCurve.hxx>
#include <GeomConvert.hxx>
#include <gp_Pnt.hxx>
#include <GeomConvert_ApproxCurve.hxx>
#include <GeomFill_Profiler.hxx>
#include <GeomFill_BSplineCurves.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Face.hxx>
#include <BRepLib.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <Precision.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <SurfTools.hxx>
#include <BSplCLib.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <vector>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <GeomFill_FillingStyle.hxx>

#ifdef DEBUG_GUIDED_SURFACE_CREATION
#include <BRepTools.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include "SplineLibIO.h"
#endif

static Standard_Real MaxTolVer(const TopoDS_Shape& aShape)
{
    Standard_Real MaxTol = 0.;

    TopExp_Explorer Explo(aShape, TopAbs_VERTEX);
    for (; Explo.More(); Explo.Next()) {
        const TopoDS_Vertex& aVertex = TopoDS::Vertex(Explo.Current());
        Standard_Real aTol = BRep_Tool::Tolerance(aVertex);
        if (aTol > MaxTol) {
            MaxTol = aTol;
        }
    }

    return MaxTol;
}

//=======================================================================
//function : MakePatches
//purpose  :
//=======================================================================

MakePatches::MakePatches()
{
    myStatus = MAKEPATCHES_OK;
}

//=======================================================================
//function : MakePatches
//purpose  :
//=======================================================================

MakePatches::MakePatches(const TopoDS_Shape& Guides,
                         const TopoDS_Shape& Profiles)
{
    Init(Guides, Profiles);
}

//=======================================================================
//function : Init
//purpose  :
//=======================================================================
void MakePatches::Init(const TopoDS_Shape& Guides,
                       const TopoDS_Shape& Profiles)
{
    myPatches.Nullify();
    myGuides = Guides;
    myProfiles = Profiles;
    myStatus = MAKEPATCHES_OK;
}

//=======================================================================
//function : GetStatus
//purpose  :
//=======================================================================
MakePatchesStatus MakePatches::GetStatus() const
{
    return myStatus;
}


//=======================================================================
//function : Perform
//purpose  :
//=======================================================================

void MakePatches::Perform(const Standard_Real theTolConf,
                          const Standard_Real theTolParam,
                          const GeomFill_FillingStyle theStyle,
                          const Standard_Boolean theSewing)
{
    // ************************************************************
    // Fuse the guides and profiles
    // ************************************************************
    BRepAlgoAPI_Fuse fuser(myGuides ,myProfiles);
    if (!fuser.IsDone()) {
        myStatus = MAKEPATCHES_FAIL_INTERSECTION;
        return;
    }
    myGrid = fuser.Shape();

    // ************************************************************
    // Get the guide and profile edges after fusing 
    // ************************************************************
    // guides as input for makeLoops
    TopTools_MapOfShape GuideEdges;
    // profiles as input for makeLoops
    TopTools_MapOfShape ProfileEdges;
    TopExp_Explorer Explo;
    TopTools_ListIteratorOfListOfShape itl;
    Explo.Init(myGuides, TopAbs_EDGE);
    for (; Explo.More(); Explo.Next()) {
        const TopoDS_Shape& anEdge = Explo.Current();
        const TopTools_ListOfShape& aList = fuser.Modified(anEdge);
        if (!aList.IsEmpty()) {
            for (itl.Initialize(aList); itl.More(); itl.Next()) {
                GuideEdges.Add(itl.Value());
            }
        }
        else {
            GuideEdges.Add(anEdge);
        }
    }
    for (Explo.Init(myProfiles, TopAbs_EDGE); Explo.More(); Explo.Next()) {
        const TopoDS_Shape& anEdge = Explo.Current();
        const TopTools_ListOfShape& aList = fuser.Modified(anEdge);
        if (!aList.IsEmpty()) {
            for (itl.Initialize(aList); itl.More(); itl.Next()) {
                ProfileEdges.Add(itl.Value());
            }
        }
        else {
            ProfileEdges.Add(anEdge);
        }
    }

#ifdef DEBUG_GUIDED_SURFACE_CREATION
    // save vertex-edge map for debugging purposes
    static int iMakePatches = 0;
    iMakePatches++;
    
    std::stringstream smygrid;
    smygrid << "makePatches_" << iMakePatches << "_wireGrid.brep";
    BRepTools::Write(myGrid, smygrid.str().c_str());

    std::stringstream sname;
    sname << "makePatches_" << iMakePatches;
#endif

    //Creating list of cells
    MakeLoops aLoopMaker(myGrid, GuideEdges, ProfileEdges);
    aLoopMaker.Perform();
    if (aLoopMaker.GetStatus() != MAKELOOPS_OK) {
        myStatus = MAKEPATCHES_FAIL_PATCHES;
        return;
    }
    const Handle(TopTools_HArray2OfShape)& PatchFrames = aLoopMaker.Cells();
    NCollection_DataMap<TopoDS_Shape, TiglContinuity> patchContinuities = aLoopMaker.Continuities();

    if (PatchFrames.IsNull()) {
        myStatus = MAKEPATCHES_FAIL_PATCHES;
        return;
    }

    BRep_Builder aBB;
    TopoDS_Compound aFaces;
    aBB.MakeCompound(aFaces);

    Standard_Integer BaseCurveIndex = 1;


    Handle(Geom_Curve) aC;
    Handle(Geom_TrimmedCurve) aTC;
    TColGeom_SequenceOfBoundedCurve aCurves;
    Standard_Real eps = theTolParam, err;
    // iterate through the two dimensional array of cell/patch boundaries
    for (Standard_Integer icell = 1; icell <= PatchFrames->ColLength(); icell++) {
        for (Standard_Integer jcell = 1; jcell <= PatchFrames->RowLength(); jcell++) {
            // ****************************************************************
            // Convert the boundaries to Geom_TrimmedCurve and leave out
            // BSpline knots which are to close to the start and end point
            // of the trimmed curve. Save the trimmed curves in 
            //
            // aCurves
            // 
            // ****************************************************************
            const TopoDS_Shape& aFrame =  PatchFrames->Value(icell, jcell);
            Standard_Real MaxTol = Max(MaxTolVer(aFrame), theTolConf);
            TopExp_Explorer anExp(aFrame, TopAbs_EDGE);
            aCurves.Clear();

#ifdef DEBUG_GUIDED_SURFACE_CREATION
            BRepBuilderAPI_MakeWire wireMaker;
#endif
            // iterate through cell boundaries
            for (; anExp.More(); anExp.Next()) {
                const TopoDS_Edge& anE = TopoDS::Edge(anExp.Current());
                Standard_Real f, l;
                aC = BRep_Tool::Curve(anE, f, l);
                Handle(Standard_Type) aType = aC->DynamicType();
                if ( aType == STANDARD_TYPE(Geom_TrimmedCurve)) {
                    aC = (*((Handle(Geom_TrimmedCurve)*)&aC))->BasisCurve();
                    aType = aC->DynamicType();
                }
                if (aType == STANDARD_TYPE(Geom_BSplineCurve)) {
                    Standard_Integer i, aNbKnots = (*((Handle(Geom_BSplineCurve)*)&aC))->NbKnots();
                    TColStd_Array1OfReal aKnots(1, aNbKnots);
                    (*((Handle(Geom_BSplineCurve)*)&aC))->Knots(aKnots);
                    Standard_Boolean isFirst = Standard_False, isLast = Standard_False;
                    gp_Pnt aPf = aC->Value(f), aPl = aC->Value(l);
                    for (i = 1; i <= aNbKnots; ++i) {
                        err = Max(Abs(eps*aKnots(i)), Precision::PConfusion());
                        if (!isFirst) {
                            if (Abs(f - aKnots(i)) <= err) {
                                gp_Pnt aP = aC->Value(aKnots(i));
                                if (aP.XYZ().IsEqual(aPf.XYZ(), theTolConf / 10.)) {
                                    f = aKnots(i);
                                    isFirst = Standard_True;
                                }
                            }
                        }
                        if (!isLast) {
                            if (Abs(l - aKnots(i)) <= err) {
                                gp_Pnt aP = aC->Value(aKnots(i));
                                if (aP.XYZ().IsEqual(aPl.XYZ(), theTolConf / 10.)) {
                                    l = aKnots(i);
                                    isLast = Standard_True;
                                }
                            }
                        }
                        if (isFirst && isLast) {
                            break;
                        }
                    }
                }

                aTC = new Geom_TrimmedCurve(aC, f, l);
                aCurves.Append(aTC);

#ifdef DEBUG_GUIDED_SURFACE_CREATION
                wireMaker.Add(BRepBuilderAPI_MakeEdge(Handle_Geom_Curve(aTC)));
#endif
            }

            // *****************************************************************************************
            // Build patch surface
            // *****************************************************************************************
            //Handle(Geom_BSplineSurface) aS = BuildSurface(aCurves, MaxTol, theTolParam, BaseCurveIndex, theStyle);
            Handle(Geom_BSplineSurface) aS;
            if (patchContinuities.Find(aFrame) == C2) {
                aS = BuildSurface(aCurves, MaxTol, theTolParam, BaseCurveIndex, theStyle);
            }
            else {
                aS = BuildSurface(aCurves, MaxTol, theTolParam, BaseCurveIndex, GeomFill_StretchStyle);
            }

#ifdef DEBUG_GUIDED_SURFACE_CREATION
            // save edges for debugging purposes
            std::stringstream siiMakePatchesInner;
            siiMakePatchesInner << sname.str() << "_curveCell_" << icell << "_" << jcell << ".brep" ;
            BRepTools::Write(wireMaker.Shape(), (siiMakePatchesInner.str()).c_str());
#endif

            if (aS.IsNull()) {
                continue;
            }

            // reparametrize surface
            if (theSewing) {
                SurfTools::UReparametrize(aS, 0., 1.);
                SurfTools::VReparametrize(aS, 0., 1.);
            }

            // convert to face
            BRepBuilderAPI_MakeFace aMkFace(aS, Precision::Confusion());

            if (!aMkFace.IsDone()) {
                continue;
            }

            TopoDS_Face aF = aMkFace.Face();
            aBB.UpdateFace(aF, theTolConf);
            BRepLib::UpdateTolerances(aF, Standard_True);

#ifdef DEBUG_GUIDED_SURFACE_CREATION
            // save edges for debugging purposes
            std::stringstream siMakePatchesInner;
            siMakePatchesInner << sname.str() << "_patchFace_" << icell << "_" << jcell;
            BRepTools::Write(aF, (siMakePatchesInner.str() + ".brep").c_str());
            Handle_Geom_BSplineSurface bsplineSurf = Handle_Geom_BSplineSurface::DownCast(BRep_Tool::Surface(aF));
            if (!bsplineSurf.IsNull()) {
                exportSurfaceToSplineLib(bsplineSurf, siMakePatchesInner.str() + ".splinelib");
            }
#endif
            // add patch to the list of patches
            aBB.Add(aFaces, aF);
        }
    }

    if (theSewing) {
        BRepBuilderAPI_Sewing aSewing(theTolConf);
        aSewing.Add(aFaces);
        aSewing.Perform();
        myPatches = aSewing.SewedShape();
    }
    else {
        myPatches = aFaces;
    }
}

//=======================================================================
//function : Patches
//purpose  :
//=======================================================================

const TopoDS_Shape& MakePatches::Patches() const
{
    return myPatches;
}

//=======================================================================
//function : BuildSurface
//purpose  :
//=======================================================================

Handle(Geom_BSplineSurface) MakePatches::BuildSurface(
    const TColGeom_SequenceOfBoundedCurve& theCurves,
    const Standard_Real theTolConf,
    const Standard_Real theTolParam,
    const Standard_Integer theBaseCurve,
    const GeomFill_FillingStyle theStyle)
{
    Standard_Integer i;
    Standard_Integer nc = theCurves.Length();

    // allow only 3 or 4 boundary curves
    if (nc != 3 && nc != 4) {
        return NULL;
    }
    // the base curve index must not be out of range
    if (theBaseCurve < 1 || theBaseCurve > nc) {
        return NULL;
    }

    // convert boundary curves to BSpline curve
    Handle(Geom_BSplineCurve) C[4];
    for ( i = 0; i < nc; i++) {
        C[i] = GeomConvert::CurveToBSplineCurve(theCurves.Value(i+1), Convert_RationalC1);
    }

    // **********************************************************************
    // Arrange boundary curves according to requirements of GeomFill_BSplineCurves:
    // 
    // 
    //                            2 
    //                  o--------->---------o 
    //                  |                   | 
    //                  |                   |
    //                  |                   |
    //                  |                   |
    //                3 ^                   ^ 1
    //                  |                   |
    //                  |                   |
    //                  |                   |
    //                  |                   |
    //                  o--------->---------o 
    //                            0 
    //                  
    //
    // The resulting boundary curves are stored in C1 and serve as input to
    // the surface generating algorithm. During this process, the input boundary
    // curves are nullified.
    // **********************************************************************
    
    // **********************************************************************
    // Set the first boundary curve to the input boundary curve with index
    // theBaseCurve - 1
    // **********************************************************************
    Standard_Real tol = theTolConf*theTolConf;
    Handle(Geom_BSplineCurve) C1[4];
    C1[0] = C[theBaseCurve-1];
    // nullify input
    C[theBaseCurve-1].Nullify();

    // **********************************************************************
    // Find the boundary curve whichs starts at the starting point of the
    // input boundary curve with index theBaseCurve - 1. Reverse direction if
    // necessary. Set this curve to the last index boundary curve C1[3].
    // Nullify the corresponding input curve afterwards.
    // **********************************************************************
    gp_Pnt aPRef = C1[0]->StartPoint();
    for (i = 1; i <= nc; ++i) {
        if (C[i-1].IsNull()) {
            continue;
        }

        gp_Pnt aP = C[i-1]->StartPoint();
        if (aP.SquareDistance(aPRef) <= tol) {
            C1[3] = C[i-1];
            C[i-1].Nullify();
            break;
        }

        aP = C[i-1]->EndPoint();
        if (aP.SquareDistance(aPRef) <= tol) {
            C[i-1]->Reverse();
            C1[3] = C[i-1];
            C[i-1].Nullify();
            break;
        }
    }

    // **********************************************************************
    // Find the boundary curve whichs starts or ends at the end point of the
    // boundary curve with index theBaseCurve - 1. 
    // Set this curve to the second index boundary curve C1[1]
    // Nullify the corresponding input curve afterwards.
    // **********************************************************************
    aPRef = C1[0]->EndPoint();
    for (i = 1; i <= nc; ++i) {
        if (C[i-1].IsNull()) {
            continue;
        }

        gp_Pnt aP = C[i-1]->StartPoint();
        if (aP.SquareDistance(aPRef) <= tol) {
            C1[1] = C[i-1];
            C[i-1].Nullify();
            break;
        }

        aP = C[i-1]->EndPoint();
        if (aP.SquareDistance(aPRef) <= tol) {
            //C[i-1]->Reverse();
            C1[1] = C[i-1];
            C[i-1].Nullify();
            break;
        }
    }

    // check if 3 initial boundary curves were set to NULL
    for (i = 0; i < 4; ++i) {
        if (!C[i].IsNull()) {
            break;
        }
    }

    if (i > 3) {
        return NULL;
    }

    // **********************************************************************
    // Set the remaining input boundary curve to the third index boundary
    // curve C1[2]
    // **********************************************************************
    C1[2] = C[i];
    C[i].Nullify();


    // **********************************************************************
    // Make sure that boundaries located opposite to each other have the same
    // degree, number of knots and poles, etc.
    // **********************************************************************
    for (i = 0; i <= 1; ++i) {
        Standard_Boolean SameDistribution = Standard_True;
        if (C1[i]->IsRational() || C1[i+2]->IsRational() ||
            C1[i]->Degree() != C1[i+2]->Degree()   ||
            C1[i]->NbPoles() != C1[i+2]->NbPoles() ||
            C1[i]->NbKnots() != C1[i+2]->NbKnots()) {

            SameDistribution = Standard_False;
        }
        else {
            //Cheking knot distribution;
            Standard_Integer nbk = C1[i]->NbKnots(), j;
            for (j = 1; j <= nbk; ++j) {
                if (C1[i]->Multiplicity(j) == C1[i+2]->Multiplicity(j)) {
                    continue;
                }
                SameDistribution = Standard_False;
                break;
            }
            //
            if (SameDistribution) {
                Standard_Real eps = theTolParam, err;
                for (j = 1; j <= nbk; ++j) {
                    err = (Abs(C1[i]->Knot(j)) + Abs(C1[i+2]->Knot(j)))/2.;
                    err = Max(eps*err, Precision::PConfusion());
                    if (Abs(C1[i]->Knot(j) - C1[i+2]->Knot(j)) < err) {
                        continue;
                    }
                    SameDistribution = Standard_False;
                    break;
                }
            }
        }
        if (SameDistribution) {
            //Set average knots for curves
            TColStd_Array1OfReal aKnots(1, C1[i]->NbKnots());
            for (Standard_Integer j = 1; j <= C1[i]->NbKnots(); ++j) {
                aKnots(j) = (C1[i]->Knot(j) + C1[i+2]->Knot(j))/2.;
            }
            C1[i]->SetKnots(aKnots);
            C1[i+2]->SetKnots(aKnots);
        }
        else {
            GeomFill_Profiler aProf;
            aProf.AddCurve(C1[i]);
            aProf.AddCurve(C1[i+2]);

            Standard_Real dtmin = RealLast();
            for (Standard_Integer j = 1; j < C1[i]->NbKnots(); ++j) {
                Standard_Real dt = C1[i]->Knot(j+1) - C1[i]->Knot(j);
                if (dt < dtmin) {
                    dtmin = dt;
                }
            }
            //
            for (Standard_Integer j = 1; j < C1[i+2]->NbKnots(); ++j) {
                Standard_Real dt = C1[i+2]->Knot(j+1) - C1[i+2]->Knot(j);
                if (dt < dtmin) {
                    dtmin = dt;
                }
            }

            Standard_Real eps = Max(dtmin*theTolParam, Precision::Confusion());
            aProf.Perform(eps);

            C1[i] = Handle(Geom_BSplineCurve)::DownCast(aProf.Curve(1));
            C1[i+2] = Handle(Geom_BSplineCurve)::DownCast(aProf.Curve(2));
        }
    }

    Standard_Integer aNbPMin = 4;
    Standard_Integer aCont = 1;
#ifdef HAVE_OCE_COONS_PATCHED
    if (theStyle == GeomFill_CoonsC2Style) {
        aNbPMin = 6;
        aCont = 2;
    }
#endif
    for (i = 0; i < 2; ++i) {
        Standard_Integer nbp = C1[i]->NbPoles();
        while (nbp < aNbPMin) {
            Standard_Integer deg = C1[i]->Degree();
            if (deg < C1[i]->MaxDegree ()) {
                C1[i]->IncreaseDegree(deg+1);
                C1[i+2]->IncreaseDegree(deg+1);
            }
            else {
                Standard_Integer j;
                Standard_Integer anIndx = 0;
                Standard_Real aKnt = -RealLast();
                for (j = C1[i]->FirstUKnotIndex(); j < C1[i]->LastUKnotIndex(); ++j) {
                    Standard_Real dt = C1[i]->Knot(j+1) - C1[i]->Knot(j);
                    if (dt > aKnt) {
                        aKnt = dt;
                        anIndx = j;
                    }
                }
                aKnt = 0.5 * (C1[i]->Knot(anIndx+1) + C1[i]->Knot(anIndx));
                C1[i]->InsertKnot (aKnt, deg - aCont);
                C1[i+2]->InsertKnot (aKnt, deg - aCont);
            }
            nbp = C1[i]->NbPoles();
        }
    }

    GeomFill_BSplineCurves aPatch;
#ifdef HAVE_OCE_COONS_PATCHED
    aPatch.Init(C1[0], C1[1], C1[2], C1[3], theStyle, theTolConf, Standard_False);
#else
    aPatch.Init(C1[0], C1[1], C1[2], C1[3], theStyle);
#endif
    const Handle(Geom_BSplineSurface)& GBS =  aPatch.Surface();

    return GBS;


}

