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

#ifdef DEBUG
#include <BRepTools.hxx>
#endif

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
    myStatus = MAKELOOPS_OK;
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
    myStatus = MAKELOOPS_OK;
}

//=======================================================================
//function : Perform
//purpose  :
//=======================================================================

void MakeLoops::Perform()
{
    TopTools_IndexedMapOfShape GridEdges;
    TopTools_IndexedDataMapOfShapeListOfShape GridVertices;

    // create map which maps the grid vertices to the neighboring edges
    TopExp::MapShapesAndAncestors(myGrid, TopAbs_VERTEX, TopAbs_EDGE, GridVertices);
    //  number of grid vertices
    Standard_Integer nbv = GridVertices.Extent();
    if (nbv == 0) {
        myStatus = MAKELOOPS_FAIL_NODATA;
        return;
    }

    // get minimum number of edges attached to a grid vertex
    Standard_Integer i, j, MinExtent = INT_MAX;
    for (i = 1; i <= GridVertices.Extent(); i++) {
        Standard_Integer anExtent = GridVertices(i).Extent();
        if (anExtent < MinExtent) {
            MinExtent = anExtent;
        }
    }

    // **********************************************************************
    // Get the global starting corner
    // **********************************************************************
    // 
    //            
    //                                 ^
    //                                 | 
    //                                 |
    //                                 |
    //                                 |
    //                     Guide Edge  |
    //                                 |
    //                                 |
    //                                 |
    //                                 o-------------------->
    //           Global Starting Corner    
    //                                      Profile Edge
    //
    // if a vertex is attached to a guide and a profile edge, both starting at
    // the vertex, it is considered to be the global starting corner.
    // global starting corner
    TopoDS_Vertex StartCorner;
    // iterate through all grid vertices which have the minimum number
    // of edges attached to them
    bool foundStartingCorner = false;
    for (i = 1; i <= GridVertices.Extent(); i++) {
        Standard_Integer anExtent = GridVertices(i).Extent();
        if (anExtent == MinExtent) {
            // set current vertex as starting corner
            StartCorner = TopoDS::Vertex(GridVertices.FindKey(i));
            Standard_Boolean ForwardSectionFound = Standard_False;
            Standard_Boolean ForwardGuideFound = Standard_False;
            // iterate through all edges attached to the current grid vertex
            TopTools_ListIteratorOfListOfShape iter(GridVertices(i));
            for (; iter.More(); iter.Next()) {
                const TopoDS_Edge& anEdge = TopoDS::Edge(iter.Value());
                // get first vertex of the edge and check if it is the
                // same as the current starting corner
                TopoDS_Vertex aFirstVertex = TopExp::FirstVertex(anEdge, Standard_True);
                if (aFirstVertex.IsSame(StartCorner)) {
                    // check if the edge is contained in the profiles
                    if (myProfileEdges.Contains(anEdge)) {
                        ForwardSectionFound = Standard_True;
                    }
                    // otherwise check if the edge is contained in the guides
                    else { //<anEdge> is in <myGuideEdges>
                        ForwardGuideFound = Standard_True;
                    }
                }
            }
            // if the current vertex is attached to a guide and a profile edge,
            // both starting at the vertex, it is considered to be the starting corner
            if (ForwardSectionFound && ForwardGuideFound) {
                foundStartingCorner = true;
                break;
            }
        }
    }
    if (!foundStartingCorner) {
        myStatus = MAKELOOPS_FAIL_STARTINGPOINT;
        return;
    }

#ifdef DEBUG
    // save vertex-edge map for debugging purposes
    static int iMakeLoops = 0;
    iMakeLoops++;
    
    std::stringstream ssedge;
    ssedge << "makeLoopPatches_" << iMakeLoops << "_startingCorner.brep";
    BRepTools::Write(StartCorner, ssedge.str().c_str());

    for (i = 1; i <= GridVertices.Extent(); i++) {
        std::stringstream svertex;
        svertex << "makeLoopPatches_" << iMakeLoops << "_vertexMap_vertex" << i << ".brep";
        BRepTools::Write(TopoDS::Vertex(GridVertices.FindKey(i)), svertex.str().c_str());
        TopTools_ListIteratorOfListOfShape iter(GridVertices(i));
        int eCount = 0;
        for (; iter.More(); iter.Next()) {
            std::stringstream sedge;
            sedge << "makeLoopPatches_" << iMakeLoops << "_vertexMap_vertex" << i << "_edge" << eCount << ".brep";
            const TopoDS_Edge& anEdge = TopoDS::Edge(iter.Value());
            BRepTools::Write(anEdge, sedge.str().c_str());
            eCount++;
        }
    }
    std::stringstream sname;
    sname << "makeLoopPatches_" << iMakeLoops;
    std::string currentName = sname.str();
#endif
    // **********************************************************************
    // Get the four sided patches
    // **********************************************************************
    BRep_Builder BB;

    // iterate through the wire to find the boundaries of four sided patches
    // NextCorner : Defines the starting corner for the next iteration
    TopoDS_Vertex NextCorner = StartCorner;
    // 
    TopoDS_Vertex NextStartCorner;
    Standard_Boolean IsFirstRow = Standard_True;
    Standard_Integer RowLength = 0;
    for (;;) {
        // the four edges of the patch
        TopoDS_Edge E1, E2, E3, E4;
        // set current starting corner as the NextCorner from the previous iteration
        TopoDS_Vertex Corner = NextCorner;
        // **********************************************************************
        // Get 1st edge of the patch
        // **********************************************************************
        //            
        //                            E1
        //                   o-------------------->
        //             Corner    
        //                        Profile Edge
        //
        // check if the current corner is attached to a profile edge which 
        // starts at the corner. If this it the case, the profile edge is
        // assigned to be the 1st edge of the patch
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
        // In case of an open section/profile the Corner might be
        // attached only to a profile edge whichs ends at the corner
        // 
        //
        //       An open profile:
        //
        //    Corner
        //          o-----<-----o
        //                      |
        //                      ^
        //                      |
        //          o----->-----o
        //   
        // in this case, set the starting corner and the global starting
        // corner to NextStartCorner and skip this iteration
        if (E1.IsNull()) { 
            if (NextStartCorner.IsNull()) {
                myStatus = MAKELOOPS_FAIL_FIRSTEDGE;
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


        // **********************************************************************
        // Get 2nd edge of the patch
        // **********************************************************************
        // 
        //            
        //                                       ^
        //                                       | 
        //                                       |
        //                                       |
        //                                       |
        //                           Guide Edge  | E2
        //                                       |
        //                                       |
        //                                       |
        //                  o--------->----------o 
        //                                        CurVertex
        //                            E1
        //
        // get the last vertex of the 1st edge of the patch and check if
        // it is attached to a guide edge which starts at the this vertex.
        // If this it the case, the guide edge is assigned to be the 2nd 
        // edge of the patch
        TopoDS_Vertex CurVertex = TopExp::LastVertex(E1, Standard_True);
        NextCorner = CurVertex;
        if (GridVertices.Contains(CurVertex)) {
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
        // break if 2nd edge of the patch was not set
        if (E2.IsNull()) { //finish
            if (myCells.IsEmpty()) {
                myStatus = MAKELOOPS_FAIL_SECONDEDGE;
            }
            break;
        }

        // **********************************************************************
        // Get 3rd edge of the patch
        // **********************************************************************
        // 
        //                            E3
        //                                        CurVertex 
        //                  o--------->----------o 
        //                                       |
        //                       Profile Edge    | 
        //                                       |
        //                                       |
        //                                       ^ E2
        //                                       | 
        //                                       |
        //                                       |
        //                                       |
        //                                       
        // get the last vertex of the 2nd edge of the patch and check if
        // it is attached to a profile edge which ends at this vertex.
        // If this it the case, the profile edge is assigned to be the 3rd
        // edge of the patch
        CurVertex = TopExp::LastVertex(E2, Standard_True);
        // If NextCorner (here: the 2nd Vertex of E1) is the same as the global starting corner,
        // the profile is closed. In this case the last vertex of E2 is set to 
        // be the current global starting corner and the starting corner of the next iteration
        // With this, the next segment is reached
        // 
        //                                 E1
        //    Global Starting Corner o------<------0            
        //    
        if (NextCorner.IsSame(StartCorner)) {
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
            if (myCells.IsEmpty()) {
                myStatus = MAKELOOPS_FAIL_THIRDEDGE;
            }
            break;
        }

        // **********************************************************************
        // Get 4th edge of the patch
        // **********************************************************************
        // 
        //                            E3
        //         CurVertex 
        //                  o--------->----------o
        //                  |
        //                  | 
        //                  |
        //                  |
        //    Guide Edge    ^ E4
        //                  | 
        //                  |
        //                  |
        //                  |
        //                  
        // get the first vertex of the 3rd edge of the patch and check if
        // it is attached to a guide edge which ends at this vertex.
        // If this it the case, the guide edge is assigned to be the 4th
        // edge of the patch
        CurVertex = TopExp::FirstVertex(E3, Standard_True);


        // if E1 is not closed, the next global starting corner is set to be
        // the first vertex of E3
        // With this, the next segment is reached
        if (Corner.IsSame(StartCorner)) {
            NextStartCorner = CurVertex;
        }

        E4.Nullify();
        if (GridVertices.Contains(CurVertex)) {
            for (iter.Initialize(GridVertices.FindFromKey(CurVertex)); iter.More(); iter.Next()) {
                E4 = TopoDS::Edge(iter.Value());
                if (myGuideEdges.Contains(E4)) {
                    TopoDS_Vertex aLastVertex = TopExp::LastVertex(E4, Standard_True);
                    TopoDS_Vertex aFirstVertex = TopExp::FirstVertex(E4, Standard_True);
                    if (aLastVertex.IsSame(CurVertex)) {
                        break;
                    }
                }
            }
        }
        if (E4.IsNull()) { //finish
            if (myCells.IsEmpty()) {
                myStatus = MAKELOOPS_FAIL_FOURTHEDGE;
            }
            break;
        }
        

        TopoDS_Compound aCell;
        BB.MakeCompound(aCell);
        BB.Add(aCell, E1);
        BB.Add(aCell, E2);
        BB.Add(aCell, E3);
        BB.Add(aCell, E4);
        myCells.Append(aCell);

#ifdef DEBUG
        static int iMakeLoopsInner = 0;
        iMakeLoopsInner++;
        // save edges for debugging purposes
        std::stringstream siMakeLoopsInner;
        siMakeLoopsInner << "_innerLoop" << iMakeLoopsInner;
        BRepTools::Write(aCell, (currentName + siMakeLoopsInner.str() +  "_cell.brep").c_str());
        BRepTools::Write(E1, (currentName + siMakeLoopsInner.str() + "_edge1.brep").c_str());
        BRepTools::Write(E2, (currentName + siMakeLoopsInner.str() + "_edge2.brep").c_str());
        BRepTools::Write(E3, (currentName + siMakeLoopsInner.str() + "_edge3.brep").c_str());
        BRepTools::Write(E4, (currentName + siMakeLoopsInner.str() + "_edge4.brep").c_str());
#endif
        //check E4 and E1 have same starting vertex
        if (TopExp::FirstVertex(E4) != TopExp::FirstVertex(E1)) {
            myStatus = MAKELOOPS_FAIL_FOURTHEDGE;
            break;
        }

        // in case of a closed section/profile the next 
        // column in the myGrid array is started
        if (NextCorner.IsSame(StartCorner)) { 
            StartCorner = NextStartCorner;
            NextCorner = StartCorner;
            if (IsFirstRow) {
                RowLength = myCells.Extent();
                IsFirstRow = Standard_False;
            }
        }
    }

    if (RowLength == 0) {
        myStatus = MAKELOOPS_FAIL_NOCLOSED_PROFILE;
        return;
    }
    // save cells segment-wise: Each column in myCellGrid represents
    // a segment
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


//=======================================================================
//function : GetStatus
//purpose  :
//=======================================================================
Standard_Integer MakeLoops::GetStatus() const
{
    return myStatus;
}
