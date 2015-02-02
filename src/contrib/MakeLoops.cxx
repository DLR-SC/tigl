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

#include <MakeLoops.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Vertex.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopoDS_Iterator.hxx>

//=======================================================================
//function : MakeLoops
//purpose  :
//=======================================================================

MakeLoops::MakeLoops()
{
}

//=======================================================================
//function : MakeLoops
//purpose  :
//=======================================================================

MakeLoops::MakeLoops(const TopoDS_Shape&         theNetWork,
                     const TopTools_MapOfShape& GuideEdges,
                     const TopTools_MapOfShape& ProfileEdges)
{
    myGrid = theNetWork;
    myGuideEdges   = GuideEdges;
    myProfileEdges = ProfileEdges;
}

//=======================================================================
//function : Init
//purpose  :
//=======================================================================

void MakeLoops::Init(const TopoDS_Shape& theNetWork,
                     const TopTools_MapOfShape& GuideEdges,
                     const TopTools_MapOfShape& ProfileEdges)
{
    myCells.Clear();
    myGrid = theNetWork;
    myGuideEdges   = GuideEdges;
    myProfileEdges = ProfileEdges;
}

//=======================================================================
//function : Perform
//purpose  :
//=======================================================================

void MakeLoops::Perform()
{
    TopTools_IndexedMapOfShape GridEdges;
    TopTools_IndexedDataMapOfShapeListOfShape GridVertices;

    TopExp::MapShapesAndAncestors(myGrid, TopAbs_VERTEX, TopAbs_EDGE, GridVertices);
    Standard_Integer nbv = GridVertices.Extent();
    if (nbv == 0) {
        return;
    }

    Standard_Integer i, j, MinExtent = 10;
    for (i = 1; i <= GridVertices.Extent(); i++) {
        Standard_Integer anExtent = GridVertices(i).Extent();
        if (anExtent < MinExtent) {
            MinExtent = anExtent;
        }
    }

    TopoDS_Vertex StartCorner;
    for (i = 1; i <= GridVertices.Extent(); i++) {
        Standard_Integer anExtent = GridVertices(i).Extent();
        if (anExtent == MinExtent) {
            StartCorner = TopoDS::Vertex(GridVertices.FindKey(i));
            Standard_Boolean ForwardSectionFound = Standard_False;
            Standard_Boolean ForwardGuideFound = Standard_False;
            TopTools_ListIteratorOfListOfShape iter(GridVertices(i));
            for (; iter.More(); iter.Next()) {
                const TopoDS_Edge& anEdge = TopoDS::Edge(iter.Value());
                TopoDS_Vertex aFirstVertex = TopExp::FirstVertex(anEdge, Standard_True);
                if (aFirstVertex.IsSame(StartCorner)) {
                    if (myProfileEdges.Contains(anEdge)) {
                        ForwardSectionFound = Standard_True;
                    }
                    else { //<anEdge> is in <myGuideEdges>
                        ForwardGuideFound = Standard_True;
                    }
                }
            }
            if (ForwardSectionFound && ForwardGuideFound) {
                break;
            }
        }
    }

    BRep_Builder BB;

    TopoDS_Vertex NextCorner = StartCorner;
    TopoDS_Vertex NextStartCorner;
    Standard_Boolean IsFirstRow = Standard_True;
    Standard_Integer RowLength = 0;
    for (;;) {
        TopoDS_Edge E1, E2, E3, E4;
        TopoDS_Vertex Corner = NextCorner;
        TopTools_ListIteratorOfListOfShape iter;
        if (GridVertices.Contains(Corner)) {
            iter.Initialize(GridVertices.FindFromKey(Corner));
            for (; iter.More(); iter.Next()) {
                const TopoDS_Edge& anEdge = TopoDS::Edge(iter.Value());
                if (myProfileEdges.Contains(anEdge)) {
                    TopoDS_Vertex aFirstVertex = TopExp::FirstVertex(anEdge, Standard_True);
                    if (aFirstVertex.IsSame(Corner)) {
                        E1 = anEdge;
                        break;
                    }
                }
            }
        }
        if (E1.IsNull()) { //in case of open section
            if (NextStartCorner.IsNull()) {
                break;
            }
            StartCorner = NextStartCorner;
            NextCorner = StartCorner;
            if (IsFirstRow) {
                RowLength = myCells.Extent();
                IsFirstRow = Standard_False;
            }
            continue;
        }

        TopoDS_Vertex CurVertex = TopExp::LastVertex(E1, Standard_True);
        NextCorner = CurVertex;
        if (GridVertices.Contains(Corner)) {
            for (iter.Initialize(GridVertices.FindFromKey(CurVertex)); iter.More(); iter.Next()) {
                const TopoDS_Edge& anEdge  = TopoDS::Edge(iter.Value());
                if (myGuideEdges.Contains(anEdge)) {
                    TopoDS_Vertex aFirstVertex = TopExp::FirstVertex(anEdge, Standard_True);
                    if (aFirstVertex.IsSame(CurVertex)) {
                        E2 = anEdge;
                        break;
                    }
                }
            }
        }
        if (E2.IsNull()) { //finish
            break;
        }

        CurVertex = TopExp::LastVertex(E2, Standard_True);
        if (NextCorner.IsSame(StartCorner)) { //in case of closed section
            StartCorner = CurVertex;
            NextCorner = StartCorner;
        }

        E3.Nullify();
        if (GridVertices.Contains(CurVertex)) {
            for (iter.Initialize(GridVertices.FindFromKey(CurVertex)); iter.More(); iter.Next()) {
                E3 = TopoDS::Edge(iter.Value());
                if (myProfileEdges.Contains(E3)) {
                    TopoDS_Vertex aLastVertex = TopExp::LastVertex(E3, Standard_True);
                    if (aLastVertex.IsSame(CurVertex)) {
                        break;
                    }
                }
            }
        }
        if (E3.IsNull()) { //finish
            break;
        }

        CurVertex = TopExp::FirstVertex(E3, Standard_True);
        if (Corner.IsSame(StartCorner)) {
            NextStartCorner = CurVertex;
        }

        E4.Nullify();
        if (GridVertices.Contains(CurVertex)) {
            for (iter.Initialize(GridVertices.FindFromKey(CurVertex)); iter.More(); iter.Next()) {
                E4 = TopoDS::Edge(iter.Value());
                if (myGuideEdges.Contains(E4)) {
                    TopoDS_Vertex aLastVertex = TopExp::LastVertex(E4, Standard_True);
                    if (aLastVertex.IsSame(CurVertex)) {
                        break;
                    }
                }
            }
        }
        if (E4.IsNull()) { //finish
            break;
        }

        TopoDS_Compound aCell;
        BB.MakeCompound(aCell);
        BB.Add(aCell, E1);
        BB.Add(aCell, E2);
        BB.Add(aCell, E3);
        BB.Add(aCell, E4);
        myCells.Append(aCell);

        if (NextCorner.IsSame(StartCorner)) { //in case of closed section
            StartCorner = NextStartCorner;
            NextCorner = StartCorner;
            if (IsFirstRow) {
                RowLength = myCells.Extent();
                IsFirstRow = Standard_False;
            }
        }
    }

    if (RowLength == 0) {
        return;
    }
    Standard_Integer ColLength = myCells.Extent() / RowLength;
    myCellGrid = new TopTools_HArray2OfShape(1, ColLength, 1, RowLength);
    i = 1;
    j = 1;
    TopTools_ListIteratorOfListOfShape itcells(myCells);
    for (; itcells.More(); itcells.Next()) {
        myCellGrid->SetValue(i, j, itcells.Value());
        j++;
        if (j > RowLength) {
            i++;
            j = 1;
        }
    }

}

//=======================================================================
//function : Cells
//purpose  :
//=======================================================================

const Handle(TopTools_HArray2OfShape)& MakeLoops::Cells() const
{
    return myCellGrid;
}

