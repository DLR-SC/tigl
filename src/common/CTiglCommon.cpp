/* 
* Copyright (C) 2012 Cassidian Air Systems
*
* Created: 2012-06-05 Roland Landertshamer
*
* Changed: $Id $ 
*
* Version: $Revision $
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

#include "CTiglCommon.h"

#include "CTiglError.h"
#include "CTiglLogging.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <vector>

#include <Bnd_Box.hxx>
#include <BOPAlgo_ArgumentAnalyzer.hxx>
#include <BOPTest_Objects.hxx>
#include <BRep_Builder.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepExtrema_ExtCF.hxx>
#include <BRepFill.hxx>
#include <BRepFill_CompatibleWires.hxx>
#include <BRepFill_Filling.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <Precision.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <GeomAPI_IntCS.hxx>
#include <Geom_Curve.hxx>
#include <GeomFill_ConstrainedFilling.hxx>
#include <Handle_GeomFill_SimpleBound.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <BRepAdaptor_HCompCurve.hxx>
#include <GeomFill_SimpleBound.hxx>
#include <Approx_Curve3d.hxx>
#include <Geom_BSplineCurve.hxx>
#include <GeomFill_BSplineCurves.hxx>
#include <GEOMAlgo_Splitter.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <ShapeFix_Wire.hxx>
#include <BRepBndLib.hxx>


namespace tigl {

	// Method for finding all directly and indirectly connected edges
	// The method loops over the passed edgeList and checks for each element if it
	// is connected to the passed edge. When an edge is found it is removed from 
	// the edgeList and added to the targetList. Additionally for this edge all
	// connected edges are also added to the targetList by recursively calling this
	// method. Finally all directly or indirectly connected edges to the passed
	// edge are moved from the edgeList to the targetList
	void CTiglCommon::findAllConnectedEdges(const TopoDS_Edge& edge, TopTools_ListOfShape& edgeList, TopTools_ListOfShape& targetList) {
		TopTools_ListIteratorOfListOfShape edgeIt;
		TopoDS_Vertex tempVertex;
		TopoDS_Edge connectedEdge;

		// finished gets true as soon as no more connected edge is found in the edgeList
		bool finished = false;
		// loop until all connected e
		while (!finished) {
			connectedEdge.Nullify();
			// iterate over all edges in edgeList, and break when a connected edge was found
			for (edgeIt.Initialize(edgeList); edgeIt.More(); edgeIt.Next()) {
				TopoDS_Edge testEdge = TopoDS::Edge(edgeIt.Value());

				// check if edges are connected
				if (checkCommonVertex(edge, testEdge)) {
					connectedEdge = testEdge;
					break;
				}
			}
			// check if a connected edge was found
			if (!connectedEdge.IsNull()) {
				// remove connected edge from edgeList
				edgeList.Remove(edgeIt);
				// append connected edge to connectedEdge list
				targetList.Append(connectedEdge);
				// append all edges which are connected to the new edge to the list
				findAllConnectedEdges(connectedEdge, edgeList, targetList);
			} else {
				finished = true;
			}
		}
	}

	TopoDS_Wire CTiglCommon::buildWire(const gp_Pnt& p1, const gp_Pnt& p2) {
		TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(p1, p2);
		TopoDS_Wire w = BRepBuilderAPI_MakeWire(e1);
		return w;
	}

	TopoDS_Face CTiglCommon::buildFace(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3, const gp_Pnt& p4) {
		BRepBuilderAPI_MakeEdge me1(p1, p2);
		TopoDS_Edge e1 = me1.Edge();
		BRepBuilderAPI_MakeEdge me2(p3, p4);
		TopoDS_Edge e2 = me2.Edge();

		TopoDS_Face face = BRepFill::Face(e1, e2);
		return face;
	}

	TopoDS_Shape CTiglCommon::cutShapes(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, bool cutSingleFaces) {
        return cutShapesInternal(shape1, shape2, cutSingleFaces, true, true, false);
	}

    TopoDS_Shape CTiglCommon::cutShapesWithExpectedResult(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, TopAbs_ShapeEnum expectedResult, bool cutSingleFaces)
    {
        TopoDS_Shape result;
        TopTools_ListOfShape resultList;
        // only for testing
        bool dump = false;
        for (int invert = 0; invert <= 1; invert++) {
            for (int usepcurve = 1; usepcurve >= 0; usepcurve--) {
                for (int approx = 1; approx >= 0; approx--) {
                    if (dump) {
                        // ONLY FOR TESTING: dump used approach into local file
                        time_t t = time(0);
                        struct tm* ts = localtime(&t);
                        std::ofstream logfile("intersectionFailures.log", std::ios_base::app);
                        logfile << "[" << (ts->tm_year+1900) << "-" << 
                                ts->tm_mon+1 << "-" << 
                                ts->tm_mday << " " << 
                                ts->tm_hour << ":" << 
                                ts->tm_min << ":" << 
                                ts->tm_sec << "] ";
                        logfile << "Retrying cut operation: approx=" << approx << " / usepcurve=" << usepcurve << " / invert=" << invert << std::endl;
                        logfile.close();
                    }
                    else {
                        dump = true;
                    }
                    result = cutShapesInternal(shape1, shape2, cutSingleFaces, approx, usepcurve, invert);
                    getListOfShape(result, expectedResult, resultList);
                    if (resultList.Extent() > 0) {
                        return result;
                    }
                }
            }
        }
        LOG(ERROR) << "Intersection computation failed: expected result shape was not found!";
        throw CTiglError("Intersection computation failed: expected result shape was not found!");
    }

    TopoDS_Shape CTiglCommon::cutShapesInternal(const TopoDS_Shape& s1, const TopoDS_Shape& s2, bool cutSingleFaces,
        bool approx, bool usepcurve, bool invert)
    {
        TopoDS_Shape result;

#ifdef VALIDATE_CUT_SHAPES
        validateCutShapes(s1, s2);
#endif

        if (cutSingleFaces) {
            TopoDS_Compound compound;
            BRep_Builder builder;
            builder.MakeCompound(compound);
            TopTools_ListOfShape faceList1, faceList2;
            if (invert) {
                getListOfShape(s2, TopAbs_FACE, faceList1);
                getListOfShape(s1, TopAbs_FACE, faceList2);
            }
            else {
                getListOfShape(s1, TopAbs_FACE, faceList1);
                getListOfShape(s2, TopAbs_FACE, faceList2);
            }
            TopTools_ListIteratorOfListOfShape faceIt1, faceIt2;

            for (faceIt1.Initialize(faceList1); faceIt1.More(); faceIt1.Next()) {
                for (faceIt2.Initialize(faceList2); faceIt2.More(); faceIt2.Next()) {
                    BRepAlgoAPI_Section splitter(faceIt1.Value(), faceIt2.Value(), Standard_False);
                    splitter.ComputePCurveOn1(usepcurve);
                    splitter.Approximation(approx);
                    splitter.Build();
                    if (splitter.IsDone()) {
                        builder.Add(compound, splitter.Shape());
                    }
                }
            }
            result = compound;
        }
        else {
            BRepAlgoAPI_Section splitter(s1, s2, Standard_False);
            splitter.ComputePCurveOn1(usepcurve);
            splitter.Approximation(approx);
            splitter.Build();
            if (!splitter.IsDone()) {
                LOG(ERROR) << "Error cutting shapes in CTiglCommon::cutShapes!";
                throw CTiglError("Error cutting shapes in CTiglCommon::cutShapes!");
            }
            result = splitter.Shape();
        }
        return result;
}

bool CTiglCommon::getIntersectionPoint(Handle(Geom_Surface) surface, Handle(Geom_Curve) curve, gp_Pnt& dst)
    {
        GeomAPI_IntCS intersection(curve, surface);
        if (intersection.IsDone()) {
            int numLines = intersection.NbSegments();
            int numPoints = intersection.NbPoints();
            if (numLines > 0) {
                LOG(WARNING) << "Intersection of curve and surface found " << numLines << " intersection lines: will be ignored!";
            }
            if (numPoints > 1) {
                LOG(WARNING) << "Intersection of curve and surface found " << numPoints << " intersection points: only first will be returned!";
            }
            if (numPoints < 1) {
                return false;
        }
        dst = intersection.Point(1);
            return true;
        }
        return false;
    }

	// Method for finding the intersection point of a face and an edge
	bool CTiglCommon::getIntersectionPoint(const TopoDS_Face& face, const TopoDS_Edge& edge, gp_Pnt& dst) {
        BRepExtrema_ExtCF edgeFaceIntersect(edge, face);
		if (!edgeFaceIntersect.IsDone()) {
            LOG(ERROR) << "Error intersecting edge and face in CTiglCommon::getIntersectionPoint!";
            throw CTiglError("Error intersecting edge and face in CTiglCommon::getIntersectionPoint!");
		}
		double minDistance = std::numeric_limits<double>::max();
		int minIndex = 0;
		for (int i=1; i <= edgeFaceIntersect.NbExt(); i++) {
			double distance = edgeFaceIntersect.SquareDistance(i);
			if (distance < minDistance) {
				minDistance = distance;
				minIndex = i;
			}
		}
		
		if (minDistance <= Precision::Confusion()) {
			dst = edgeFaceIntersect.PointOnEdge(minIndex);
			return true;
		}
		return false;
	}

	// Method for finding the intersection point of a face and a wire (containing edges)
	bool CTiglCommon::getIntersectionPoint(const TopoDS_Face& face, const TopoDS_Wire& wire, gp_Pnt& dst) {
		BRepTools_WireExplorer wireExp;
		for (wireExp.Init(wire); wireExp.More(); wireExp.Next()) {
			const TopoDS_Edge& edge = wireExp.Current();
			if (CTiglCommon::getIntersectionPoint(face, edge, dst)) {
				return true;
			}
		}
		return false;
	}


	TopoDS_Wire CTiglCommon::makeWireFromEdges(const TopoDS_Shape& edges) {
		TopTools_ListOfShape wireList;
		makeWiresFromConnectedEdges(edges, wireList);
		if (wireList.Extent() != 1) {
            LOG(ERROR) << "Error generating single wire!";
            throw CTiglError("Error generating single wire in CTiglCommon::makeWireFromEdges!");
		}
		TopoDS_Wire result = TopoDS::Wire(wireList.First());
		return result;
	}

	void CTiglCommon::getListOfShape(const TopoDS_Shape& shape, TopAbs_ShapeEnum type, TopTools_ListOfShape& result) {
		TopTools_IndexedMapOfShape typeMap;
		TopExp::MapShapes(shape, type, typeMap);
		for (int i = 1; i <= typeMap.Extent(); i++) {
			result.Append(typeMap.FindKey(i));
		}
	}

	// Method for getting all wires from the passed shape (list of edges)
	void CTiglCommon::makeWiresFromConnectedEdges(const TopoDS_Shape& shape, TopTools_ListOfShape& wireList) {
		
		// get list of edges from passed shape
		TopTools_ListOfShape edgeList;
		getListOfShape(shape, TopAbs_EDGE, edgeList);

		// iterate until all edges are converted into wires
		while (edgeList.Extent() > 0) {
			// pop first edge from edgeList
			TopoDS_Edge e1 = TopoDS::Edge(edgeList.First());
			edgeList.RemoveFirst();
			// build a list of edges for a single wire
			TopTools_ListOfShape wireEdges;
			wireEdges.Append(e1);
			findAllConnectedEdges(e1, edgeList, wireEdges);

			// build wire from wireEdges
			BRepBuilderAPI_MakeWire wireBuilder;
			TopTools_ListIteratorOfListOfShape eIt;
			for (eIt.Initialize(wireEdges); eIt.More(); eIt.Next())
			{
				wireBuilder.Add(TopoDS::Edge(eIt.Value()));
			}
			if (!wireBuilder.IsDone()) {
                LOG(ERROR) << "Error creating wire!";
                throw CTiglError("Error creating wire in CTiglCommon::makeWiresFromConnectedEdges!");
			}
			TopoDS_Wire wire = sortWireEdges(wireBuilder.Wire(), false);

			wireList.Append(wire);
		}
	}

    TopoDS_Shape CTiglCommon::makeSortedEdgeShape(const TopoDS_Shape& shape) {
        TopTools_ListOfShape wireList;
        TopTools_ListIteratorOfListOfShape it;
        BRep_Builder builder;
        TopoDS_Compound result;

        // initialize resulting shape
        builder.MakeCompound(result);
        // generate a list of wires (with sorted edges)
        makeWiresFromConnectedEdges(shape, wireList);
        // combine wires again to single compound
        for (it.Initialize(wireList); it.More(); it.Next())
        {
            builder.Add(result, TopoDS::Wire(it.Value()));
        }
        return result;
    }

	void CTiglCommon::getEndVertices(const TopoDS_Shape& shape, TopTools_ListOfShape& endVertices) {
		// get a map of vertices with connected edges
		TopTools_IndexedDataMapOfShapeListOfShape vertexMap;
		TopExp::MapShapesAndAncestors(shape, TopAbs_VERTEX, TopAbs_EDGE, vertexMap);

		// filter out all vertices which have only a single edge connected (end vertices)
		for (int i = 1; i <= vertexMap.Extent(); i++) {
			const TopTools_ListOfShape& edgeList = vertexMap.FindFromIndex(i);
			if (edgeList.Extent() == 1) {
				endVertices.Append(vertexMap.FindKey(i));
			}
		}
	}

	TopoDS_Wire CTiglCommon::closeWire(const TopoDS_Wire& wire) {
		// get the list of end vertices
		TopTools_ListOfShape endVertices;
		getEndVertices(wire, endVertices);

		// determine number of end vertices
		int numEndVertices = endVertices.Extent();

		// check if wire is already closed
		if (numEndVertices == 0) {
			return wire;
		}

		// check if we have exatcly two end vertices
		if (numEndVertices != 2) {
            LOG(ERROR) << "invalid number of end vertices found!";
            throw CTiglError("Error: invalid number of end vertices found in CTiglCommon::closeWire!");
		}

		// next generate an edge between the end vertices
		TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(TopoDS::Vertex(endVertices.First()), TopoDS::Vertex(endVertices.Last()));

		// build new wire
		BRepBuilderAPI_MakeWire makeWire;
		makeWire.Add(wire);
		makeWire.Add(edge);
		if (!makeWire.IsDone()) {
            LOG(ERROR) << "unable to build closed wire!";
            throw CTiglError("Error: unable to build closed wire in CTiglCommon::closeWire!");
		}
		TopoDS_Wire result = sortWireEdges(makeWire.Wire(), true);
		return result;
	}

    TopoDS_Face CTiglCommon::buildFace(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2) {
        TopoDS_Wire closedWire = closeWires(wire1, wire2);
        BRepBuilderAPI_MakeFace mf(closedWire);
        TopoDS_Face face;
        if (mf.IsDone())
        {
            face = mf.Face();
        }
        else
        {
            face = buildRuledFace(wire1, wire2);
        }
        return face;
    }

	TopoDS_Face CTiglCommon::buildFace(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2, const gp_Vec& dir) {
		TopoDS_Wire closedWire = closeWires(wire1, wire2, dir);
        BRepBuilderAPI_MakeFace mf(closedWire);
        TopoDS_Face face;
        if (mf.IsDone())
        {
            face = mf.Face();
        }
        else
        {
            face = buildRuledFace(wire1, wire2);
        }
        return face;
	}

    TopoDS_Face CTiglCommon::buildFace(const TopoDS_Wire& wire) {
        TopoDS_Face result;

        // try building face using BRepLib_MakeFace, which tries to build a plane
        // if a plane is not possible, use BRepFill_Filling
        BRepLib_MakeFace mf(wire, Standard_True);
        if (mf.IsDone()) {
            result = mf.Face();
        }
        else {
            BRepFill_Filling filler;
            TopExp_Explorer exp;
            for (exp.Init(wire, TopAbs_EDGE); exp.More(); exp.Next()) {
                TopoDS_Edge e = TopoDS::Edge(exp.Current());
                filler.Add(e, GeomAbs_C0);
            }
            filler.Build();
            if (!filler.IsDone()) {
                LOG(ERROR) << "Unable to generate face from Wire!";
                throw CTiglError("CTiglCommon::buildFace: Unable to generate face from Wire!");
            }
            result = filler.Face();
        }
        return result;
    }

	// determine direction from wire1 by using end vertices
	TopoDS_Wire CTiglCommon::closeWires(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2) {
		// get a map of vertices with connected edges
		TopTools_IndexedDataMapOfShapeListOfShape vertexMap;
		TopTools_ListOfShape endVertices1, endVertices2;

		TopExp::MapShapesAndAncestors(wire1, TopAbs_VERTEX, TopAbs_EDGE, vertexMap);
		// filter out all vertices which have only a single edge connected (end vertices)
		for (int i = 1; i <= vertexMap.Extent(); i++) {
			const TopTools_ListOfShape& edgeList = vertexMap.FindFromIndex(i);
			if (edgeList.Extent() == 1) {
				endVertices1.Append(vertexMap.FindKey(i));
			}
		}

		// check for correct number of end vertices
		if (endVertices1.Extent() != 2) {
            LOG(ERROR) << "Unable to close wires because invalid number of end-vertices found!";
            throw CTiglError("Error: Unable to close wires because invalid number of end-vertices found!");
		}

		TopoDS_Vertex& v1 = TopoDS::Vertex(endVertices1.First());
		TopoDS_Vertex& v2 = TopoDS::Vertex(endVertices1.Last());
		gp_Pnt p1 = BRep_Tool::Pnt(v1);
		gp_Pnt p2 = BRep_Tool::Pnt(v2);
		gp_Vec dir(p1, p2);

		return closeWires(wire1, wire2, dir);
	}

	TopoDS_Wire CTiglCommon::closeWires(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2, const gp_Vec& dir) {
		// get a map of vertices with connected edges
		TopTools_IndexedDataMapOfShapeListOfShape vertexMap;
		TopTools_ListOfShape endVertices1, endVertices2;

		TopExp::MapShapesAndAncestors(wire1, TopAbs_VERTEX, TopAbs_EDGE, vertexMap);
		// filter out all vertices which have only a single edge connected (end vertices)
		for (int i = 1; i <= vertexMap.Extent(); i++) {
			const TopTools_ListOfShape& edgeList = vertexMap.FindFromIndex(i);
			if (edgeList.Extent() == 1) {
				endVertices1.Append(vertexMap.FindKey(i));
			}
		}
		vertexMap.Clear();
		TopExp::MapShapesAndAncestors(wire2, TopAbs_VERTEX, TopAbs_EDGE, vertexMap);
		// filter out all vertices which have only a single edge connected (end vertices)
		for (int i = 1; i <= vertexMap.Extent(); i++) {
			const TopTools_ListOfShape& edgeList = vertexMap.FindFromIndex(i);
			if (edgeList.Extent() == 1) {
				endVertices2.Append(vertexMap.FindKey(i));
			}
		}

		// check if number of end vertices is correct
		int numEndVertices1 = endVertices1.Extent();
		int numEndVertices2 = endVertices2.Extent();
		if (numEndVertices1 != 2 || numEndVertices2 != 2) {
            LOG(ERROR) << "Unable to close wires because invalid number of end-vertices found!";
            throw CTiglError("Error: Unable to close wires because invalid number of end-vertices found!");
		}

		// sort end vertices according to direction vector
		TopoDS_Vertex vUpper1 = TopoDS::Vertex(endVertices1.First());
		TopoDS_Vertex vLower1 = TopoDS::Vertex(endVertices1.Last());
		gp_Pnt pUpper = BRep_Tool::Pnt(vUpper1);
		gp_Pnt pLower = BRep_Tool::Pnt(vLower1);
		gp_Vec vDiff(pLower, pUpper);
		if (vDiff.Normalized().Dot(dir.Normalized()) < 0) {
			vUpper1 = TopoDS::Vertex(endVertices1.Last());
			vLower1 = TopoDS::Vertex(endVertices1.First());
		}
		TopoDS_Vertex vUpper2 = TopoDS::Vertex(endVertices2.First());
		TopoDS_Vertex vLower2 = TopoDS::Vertex(endVertices2.Last());
		pUpper = BRep_Tool::Pnt(vUpper2);
		pLower = BRep_Tool::Pnt(vLower2);
		vDiff = gp_Vec(pLower, pUpper);
		if (vDiff.Normalized().Dot(dir.Normalized()) < 0) {
			vUpper2 = TopoDS::Vertex(endVertices2.Last());
			vLower2 = TopoDS::Vertex(endVertices2.First());
		}

		// next build the edges and combine the wires to a single wire
		TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(vUpper1, vUpper2);
		TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(vLower1, vLower2);

		// put all edges into a large list
		TopTools_ListOfShape edgeList;
		edgeList.Append(edge1);
		edgeList.Append(edge2);
		TopoDS_Iterator edgeIt;
		for (edgeIt.Initialize(wire1); edgeIt.More(); edgeIt.Next()) {
			edgeList.Append(edgeIt.Value());
		}
		for (edgeIt.Initialize(wire2); edgeIt.More(); edgeIt.Next()) {
			edgeList.Append(edgeIt.Value());
		}

		BRepBuilderAPI_MakeWire makeWire;
		makeWire.Add(edgeList);
		if (!makeWire.IsDone()) {
            LOG(ERROR) << "error during creation of closed wire!";
            throw CTiglError("Error: error during creation of closed wire in CTiglCommon::closeWires!");
		}

		TopoDS_Wire result = sortWireEdges(makeWire.Wire(), true /* closed */);

		return result;
	}

    // NOTE: parameter closed is deprecated, seems to be not required by ShapeFix_Wire
    TopoDS_Wire CTiglCommon::sortWireEdges(const TopoDS_Wire& wire, bool closed) {
        ShapeFix_Wire fixWire;
        fixWire.Load(wire);
        fixWire.FixReorder();
        TopoDS_Wire result = fixWire.Wire();
        return result;
    }

	// Method for checking if two edges have a common vertex (same position)
	bool CTiglCommon::checkCommonVertex(const TopoDS_Edge& e1, const TopoDS_Edge& e2) {
		TopoDS_Vertex v1First, v1Last, v2First, v2Last;
		TopExp::Vertices(e1, v1First, v1Last);
		TopExp::Vertices(e2, v2First, v2Last);

		gp_Pnt p1First = BRep_Tool::Pnt(v1First);
		gp_Pnt p1Last = BRep_Tool::Pnt(v1Last);
		gp_Pnt p2First = BRep_Tool::Pnt(v2First);
		gp_Pnt p2Last = BRep_Tool::Pnt(v2Last);

		if (p1First.Distance(p2First) < Precision::Confusion() ||
			p1First.Distance(p2Last) < Precision::Confusion() ||
			p1Last.Distance(p2First) < Precision::Confusion() ||
			p1Last.Distance(p2Last) < Precision::Confusion()) {
			return true;
		} else {
			return false;
		}
	}


	// Method for checking for duplicate edges in the passed shape.
	// The method returns a shape with only unique edges
	// NOTE: THIS METHOD ONLY CHECKS THE VERTEX POSITIONS, AND THE MIDDLE POINT 
	//       OF THE EDGES, BUT DOES NOT COMPARE THE CURVES EXACTLY
	TopoDS_Shape CTiglCommon::removeDuplicateEdges(const TopoDS_Shape& shape) {
		TopTools_ListOfShape initialEdgeList, newEdgeList;
		TopTools_ListIteratorOfListOfShape initialEdgeIt, newEdgeIt;

		TopoDS_Vertex v1First, v1Last, v2First, v2Last;
		gp_Pnt p1First, p1Mid, p1Last, p2First, p2Mid, p2Last;
		
		// get list of all edges of passed shape
		getListOfShape(shape, TopAbs_EDGE, initialEdgeList);

		// iterate over all edges
		for (initialEdgeIt.Initialize(initialEdgeList); initialEdgeIt.More(); initialEdgeIt.Next())
		{
			TopoDS_Edge& edge = TopoDS::Edge(initialEdgeIt.Value());
			TopExp::Vertices(edge, v1First, v1Last);
			p1First = BRep_Tool::Pnt(v1First);
			p1Last = BRep_Tool::Pnt(v1Last);

			// if identical edge is not found already in newEdgeList then add it to this list
			bool duplicate = false;
			for (newEdgeIt.Initialize(newEdgeList); newEdgeIt.More(); newEdgeIt.Next())
			{
				TopoDS_Edge& checkEdge = TopoDS::Edge(newEdgeIt.Value());
				TopExp::Vertices(checkEdge, v2First, v2Last);
				p2First = BRep_Tool::Pnt(v2First);
				p2Last = BRep_Tool::Pnt(v2Last);

				if ((p1First.Distance(p2First) < Precision::Confusion() &&
					 p1Last.Distance(p2Last) < Precision::Confusion()) ||
					(p1First.Distance(p2Last) < Precision::Confusion() &&
					p1Last.Distance(p2First) < Precision::Confusion())) 
				{
					// now check for identical mid point
					Handle(Geom_Curve) curve;
					Standard_Real uStart, uEnd;
					// get midpoint of edge
					curve = BRep_Tool::Curve(edge, uStart, uEnd);
					curve->D0((uStart+uEnd)/2.0, p1Mid);
					// get midpoint of checkEdge
					curve = BRep_Tool::Curve(checkEdge, uStart, uEnd);
					curve->D0((uStart+uEnd)/2.0, p2Mid);

					// because of bug #505 i had to increase the maximum allowed
					// distance for midpoints
					if (p1Mid.Distance(p2Mid) < 1E-5)
					{
						duplicate = true;
						break;
					}
				}
			}
			// only add edge to newEdgeList if no identical edge already exists in there
			if (!duplicate) 
			{
				newEdgeList.Append(edge);
			}
		}

		// finally rebuild a compound of all edges from the newEdgeList
		TopoDS_Compound result;
		BRep_Builder builder;
		builder.MakeCompound(result);

		for (newEdgeIt.Initialize(newEdgeList); newEdgeIt.More(); newEdgeIt.Next())
		{
			builder.Add(result, TopoDS::Edge(newEdgeIt.Value()));
		}

		return result;
	}

	void CTiglCommon::dumpShape(const TopoDS_Shape& shape, std::string prefix, std::string fileName, int counter) {
		std::stringstream ss;
		ss << prefix << "/" << fileName;
		if (counter >= 0) {
			ss << counter;
		}
		ss << ".brep";
		std::string file = ss.str();
		BRepTools::Write(shape, file.c_str());
	}

	// Helper Function for limiting double to range.
	// Throws exception when difference to min/max value is larger then Precision
	double CTiglCommon::limitRange(double d, double min, double max)
	{
		if ((d - max) > Precision::Confusion() || (d - min) < -Precision::Confusion()) 
		{
            LOG(ERROR) << "INTERNAL ERROR: parameter for method CTiglCommon::limit exceeds precision!";
            throw CTiglError("INTERNAL ERROR: parameter for method CTiglCommon::limit exceeds precision!");
		}
		if (d > max)
			return max;
		else if (d < min)
			return min;
		return d;
	}

	double CTiglCommon::getAngleBetweenEdges(const TopoDS_Edge& edge1, const TopoDS_Edge& edge2)
	{
		Standard_Real umin1, umax1, umin2, umax2;
		
		// get curve and u-coordinates for edges
		Handle(Geom_Curve) c1 = BRep_Tool::Curve(edge1, umin1, umax1);
		Handle(Geom_Curve) c2 = BRep_Tool::Curve(edge2, umin2, umax2);

		// determine connection vertex/point between edges
		TopoDS_Vertex vertex = TopExp::LastVertex(edge1);
		TopoDS_Vertex vCheck1 = TopExp::FirstVertex(edge2);
		TopoDS_Vertex vCheck2 = TopExp::LastVertex(edge2);
		gp_Pnt point = BRep_Tool::Pnt(vertex);
		gp_Pnt pCheck1 = BRep_Tool::Pnt(vCheck1);
		gp_Pnt pCheck2 = BRep_Tool::Pnt(vCheck2);

		if (!point.IsEqual(pCheck1, Precision::Confusion()) && !point.IsEqual(pCheck2, Precision::Confusion()))
		{
			vertex = TopExp::FirstVertex(edge1);
			point = BRep_Tool::Pnt(vertex);
		}

		// ensure that both edges are really connected!!!
		if (!point.IsEqual(pCheck1, Precision::Confusion()) && !point.IsEqual(pCheck2, Precision::Confusion()))
		{
            LOG(ERROR) << "passed edges are not connected!";
            throw CTiglError("ERROR: CTiglCommon::getAngleBetweenEdges(): passed edges are not connected!");
		}

		// get direction of curve 1 at connection point
		gp_Pnt pCheck;
		gp_Vec dir1, dir2, dirUmin, dirUmax;
		Standard_Real distUmin, distUmax;

		c1->D1(umin1, pCheck, dirUmin);
		distUmin = pCheck.Distance(point);
		c1->D1(umax1, pCheck, dirUmax);
		distUmax = pCheck.Distance(point);
		if (distUmin < distUmax)
		{
			// inverse direction vector from incoming vector in case the curve
			// has different direction
			dir1 = gp_Vec(dirUmin).Multiplied(-1);
		}
		else
		{
			dir1 = gp_Vec(dirUmax);
		}

		// get direction of curve 2 at connection point
		c2->D1(umin2, pCheck, dirUmin);
		distUmin = pCheck.Distance(point);
		c2->D1(umax2, pCheck, dirUmax);
		distUmax = pCheck.Distance(point);
		if (distUmin < distUmax)
		{
			dir2 = gp_Vec(dirUmin);
		}
		else
		{
			// inverse direction vector from outgoing vector in case the curve
			// has different direction
			dir2 = gp_Vec(dirUmax).Multiplied(-1);
		}
		// compute angle between edges in connection point
		double angle = dir1.Angle(dir2);
		return angle;
	}

	// splits wire of face into 4 separate wires, representing the boundaries of the face
	void CTiglCommon::splitFaceWires(const TopoDS_Wire& wire, TopTools_ListOfShape& wireList)
	{
		std::vector<double> edgeAngles;
		std::vector<TopoDS_Wire> wires;
		Standard_Real umin1, umax1, umin2, umax2, uNext;
		gp_Pnt p, pCheck;
		gp_Vec dir1, dir2, dirUmax, dirUmin;
		TopoDS_Edge e1, e2, startEdge;
		Handle(Geom_Curve) c1, c2;
		double distUmax, distUmin;

		// build wire explorer for iteration over connected edge loop and initialize first edge
		BRepTools_WireExplorer wireExp(wire);
		e1 = wireExp.Current();
		while (isLoopingEdge(e1) && wireExp.More())
		{
			wireExp.Next();
			e1 = wireExp.Current();
		}
		if (!wireExp.More())
		{
            LOG(ERROR) << "no valid edge found in wire!";
            throw CTiglError("ERROR: CTiglCommon::splitFaceWires(): no valid edge found in wire!");
		}
		c1 = BRep_Tool::Curve(e1, umin1, umax1);
		// store start edge for last angle computation
		startEdge = e1;

		// add edge to wire list
		wires.push_back(BRepBuilderAPI_MakeWire(e1));

		// continue with second edge
		wireExp.Next();
		if (!wireExp.More())
		{
            LOG(ERROR) << "only single edge in wire found!";
            throw CTiglError("ERROR: CTiglCommon::splitFaceWires(): only single edge in wire found!");
		}

		// TODO: USE getAngleBetweenEdges routine for angle computation!!!
		bool firstRun = true;
		// iterate over all connected edges in order
		for (; wireExp.More(); wireExp.Next())
		{
			// get next edge and curve of edge
			e2 = wireExp.Current();
			// skip looping edges
			if (isLoopingEdge(e2))
			{
				continue;
			}
			c2 = BRep_Tool::Curve(e2, umin2, umax2);
			
			// Determine connection point between edges
			const TopoDS_Vertex& v = wireExp.CurrentVertex();
			p = BRep_Tool::Pnt(v);
	
			// compute correct u value only on first run, on preceding use uNext
			if (firstRun)
			{
				// get direction of curve 1 at connection point
				c1->D1(umin1, pCheck, dirUmin);
				distUmin = pCheck.Distance(p);
				c1->D1(umax1, pCheck, dirUmax);
				distUmax = pCheck.Distance(p);
				if (distUmin < distUmax)
				{
					// inverse direction vector from incoming vector in case the 
					// curve has different direction
					dir1 = dirUmin.Multiplied(-1);
				}
				else
				{
					dir1 = dirUmax;
				}
				firstRun = false;
			}
			else
			{
				c1->D1(uNext, pCheck, dir1);
				// inverse direction vector from incoming vector in case the 
				// curve has different direction
				if (uNext == umin1)
				{
					dir1.Multiply(-1);
			}
			}

			// get direction of curve 2 at connection point
			c2->D1(umin2, pCheck, dirUmin);
			distUmin = pCheck.Distance(p);
			c2->D1(umax2, pCheck, dirUmax);
			distUmax = pCheck.Distance(p);
			if (distUmin < distUmax)
			{
				dir2 = dirUmin;
				uNext = umax2;
			}
			else
			{
				// inverse direction vector from incoming vector in case the 
				// curve has different direction
				dir2 = dirUmax.Multiplied(-1);
				uNext = umin2;
			}
			
			// compute angle between edges in connection point
			double angle = dir1.Angle(dir2);
			edgeAngles.push_back(angle);

			// add edge to wire list
			wires.push_back(BRepBuilderAPI_MakeWire(e2));

			// prepare for next iteration
			e1 = e2;
			c1 = c2;
			umin1 = umin2;
			umax1 = umax2;
		}

		// finally compute angle between last edge in loop and first edge
		e2 = startEdge;
		c2 = BRep_Tool::Curve(e2, umin2, umax2);
		c1->D1(uNext, p, dir1);
		c2->D1(umin2, pCheck, dirUmin);
		distUmin = pCheck.Distance(p);
		c2->D1(umax2, pCheck, dirUmax);
		distUmax = pCheck.Distance(p);
		if (distUmin < distUmax)
		{
			dir2 = dirUmin;
		}
		else
		{
			dir2 = dirUmax;
		}

		double angle = dir1.Angle(dir2);
		edgeAngles.push_back(angle);

		// iterate until we have only 4 wires left
		while (wires.size() > 4) 
		{
			// find index of minimum angle
			int minIndex = 0;
			for (int i=1; i < edgeAngles.size(); i++) 
			{
				if (edgeAngles[i] < edgeAngles[minIndex])
				{
					minIndex = i;
				}
			}

			// combine wires which have minimum angle difference
			TopoDS_Wire w1 = wires[minIndex];
			int nextIndex = (minIndex + 1) % wires.size();
			TopoDS_Wire w2 = wires[nextIndex];
			BRepBuilderAPI_MakeWire mw;
			mw.Add(w1);
			mw.Add(w2);
			// replace second wire with combined one
			wires[nextIndex] = mw.Wire();
			// remove first wire
			wires.erase(wires.begin() + minIndex);
			// remove minimum angle from list
			edgeAngles.erase(edgeAngles.begin() + minIndex);
		}

		// copy over combined wires into wire list
		for (int i=0; i < wires.size(); i++)
		{
			wireList.Append(wires[i]);
		}
	}

	TopoDS_Face CTiglCommon::fillWireWithNurbsSurface(const TopoDS_Wire& wire)
	{
		TopTools_ListOfShape wireList;
		// split up wires for getting maximum of 4 face boundaries
		splitFaceWires(wire, wireList);
		return fillWireListWithNurbsSurface(wireList);
	}

	TopoDS_Face CTiglCommon::fillWireListWithNurbsSurface(const TopTools_ListOfShape& wireList)
	{
		static const int maxSegments = 2000;
		static const int maxDegree = 12;
		TopoDS_Face result;
		TopTools_ListIteratorOfListOfShape wireIt;
		// vector of bsplines used for generation of surface out of two curves
		std::vector<Handle(Geom_BSplineCurve)> bsplines;
		// boundaries used for generation of surface out of three or four curves
		std::vector<Handle(GeomFill_SimpleBound)> bounds;
		Handle(Geom_BSplineSurface) surface;

		// iterate over returned wires and build approximating bspline curve for each
		for (wireIt.Initialize(wireList); wireIt.More(); wireIt.Next())
		{
			TopoDS_Wire w = TopoDS::Wire(wireIt.Value());
			BRepAdaptor_CompCurve compCurve(w, Standard_False);
			Handle(Adaptor3d_HCurve) curve = new BRepAdaptor_HCompCurve(compCurve);
			if (wireList.Extent() == 2)
			{
				Approx_Curve3d curveApprox(curve, Precision::Confusion(), GeomAbs_C0, maxSegments, maxDegree);
				if (!curveApprox.IsDone())
				{
                    LOG(ERROR) << "unable to approximate wire with curve in fillWireWithNurbsSurface";
                    throw CTiglError("ERROR: unable to approximate wire with curve in fillWireWithNurbsSurface");
				}
				bsplines.push_back(curveApprox.Curve());
			}
			else
			{
				Handle(GeomFill_SimpleBound) simpleBound = new GeomFill_SimpleBound(curve, Precision::Confusion(), Precision::Confusion());
				bounds.push_back(simpleBound);
			}
		}
		// execute surface generation
		if (wireList.Extent()  == 2)
		{
			// ensure that both bsplines start at same point, otherwise 
			// generated surface will be incorrect
			Handle_Geom_BSplineCurve bspline1 = bsplines[0];
			Handle_Geom_BSplineCurve bspline2 = bsplines[1];
			gp_Pnt ps1 = bspline1->StartPoint();
			gp_Pnt ps2 = bspline2->StartPoint();
			gp_Pnt pe2 = bspline2->EndPoint();
			if (ps1.SquareDistance(ps2) > ps1.SquareDistance(pe2))
			{
				bspline2->Reverse();
			}

			GeomFill_BSplineCurves filler(bspline1, bspline2, GeomFill_CoonsStyle);
			surface = filler.Surface();
		}
		else if (wireList.Extent() == 3)
		{
			GeomFill_ConstrainedFilling filler(maxDegree, maxSegments);
			filler.Init(bounds[0], bounds[1], bounds[2]);
			surface = filler.Surface();
		}
		else if (wireList.Extent() == 4)
		{
			GeomFill_ConstrainedFilling filler(maxDegree, maxSegments);
		filler.Init(bounds[0], bounds[1], bounds[2], bounds[3]);
			surface = filler.Surface();
		}
		else
		{
            LOG(ERROR) << "invalid number of curves found for filling wire with nurbs surface!";
            throw CTiglError("ERROR: invalid number of curves found for filling wire with nurbs surface!");
		}
		// build face from surface
		result = BRepBuilderAPI_MakeFace(surface, Precision::Confusion());
		return result;
	}

        // moved from CTIGLViewerDocument
    // [[CAS_AES]] added method for mirroring geometry
    gp_Pnt CTiglCommon::mirrorPoint(gp_Pnt pnt, TiglSymmetryAxis symmetryAxis)
    {
        if (symmetryAxis == TIGL_X_Y_PLANE) {
            pnt.SetZ(-pnt.Z());
        }
        else if (symmetryAxis == TIGL_X_Z_PLANE) {
            pnt.SetY(-pnt.Y());
        }
        else if (symmetryAxis == TIGL_Y_Z_PLANE) {
            pnt.SetX(-pnt.X());
        }

        return pnt;
    }

        // moved from CTIGLViewerDocument
    // [[CAS_AES]] added method for mirroring geometry
    TopoDS_Shape CTiglCommon::mirrorShape(const TopoDS_Shape& shape, TiglSymmetryAxis symmetryAxis, bool retBoth)
    {
        if (symmetryAxis == TIGL_X_Y_PLANE) {
            return mirrorShape(shape, gp_Vec(0, 0, 1), retBoth);
        } else if (symmetryAxis == TIGL_X_Z_PLANE) {
            return mirrorShape(shape, gp_Vec(0, 1, 0), retBoth);
        } else if (symmetryAxis == TIGL_Y_Z_PLANE) {
            return mirrorShape(shape, gp_Vec(1, 0, 0), retBoth);
        } else {
            return shape;
        }
    }

    TopoDS_Compound CTiglCommon::mirrorShape(const TopoDS_Shape& shape, gp_Vec direction, bool retBoth)
    {
        gp_Ax2 mirrorPlane(gp_Pnt(0,0,0),gp_Dir(direction.Normalized()));

        gp_Trsf mirrorTrans;
        mirrorTrans.SetMirror(mirrorPlane);
        BRepBuilderAPI_Transform trans(shape, mirrorTrans, Standard_True);
        TopoDS_Shape mirrorShape = trans.Shape();
    
        TopoDS_Compound compound;
        BRep_Builder builder;
        builder.MakeCompound(compound);

        if (retBoth)
            builder.Add(compound, shape);

        builder.Add(compound, mirrorShape);
        return compound;
    }

    void CTiglCommon::trimWing( TopoDS_Shape& nWing, TopoDS_Shape& nFuselage)
    {

        // Bounding Box for the fuselage
        Bnd_Box FuselageBBox;
        Bnd_Box WingBBox;
        BRepBndLib::Add(nFuselage, FuselageBBox);

        // Compund for returned wing
        TopoDS_Compound WingComp;
        TopoDS_Compound FuselageComp;
        BRep_Builder builder;
        builder.MakeCompound(WingComp);
        builder.MakeCompound(FuselageComp);
        // cut the wing with the fuselage
        TopoDS_Shape splittedWing = splitShape(nWing, nFuselage);

//         get the faces of the cutted wing

        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(splittedWing, TopAbs_FACE, faceMap);

        for (int f = 1; f <= faceMap.Extent(); f++) 
        {
    
            TopoDS_Shape loftFace = TopoDS::Face(faceMap(f));

            Bnd_Box tmpBBox;
            BRepBndLib::Add(loftFace, tmpBBox);

            if (FuselageBBox.IsOut(tmpBBox))
            {
                builder.Add(WingComp, loftFace);
            }
        }

        nWing = WingComp;

        BRepBndLib::Add(nWing, WingBBox);

        TopoDS_Shape splittedFuselage = splitShape(nFuselage, nWing);

//         get the faces of the cutted wing

        TopExp::MapShapes(splittedFuselage, TopAbs_FACE, faceMap);

        for (int f = 1; f <= faceMap.Extent(); f++) 
        {
    
            TopoDS_Shape loftFace = TopoDS::Face(faceMap(f));

            Bnd_Box tmpBBox;
            BRepBndLib::Add(loftFace, tmpBBox);

            if (WingBBox.IsOut(tmpBBox))
            {
                builder.Add(FuselageComp, loftFace);
            }
        }

        nFuselage = FuselageComp;

    }



	// iterate over all faces and generate NURBS surfaces
	TopoDS_Shape CTiglCommon::buildNurbsFaces(const TopoDS_Shape& shape)
	{
		TopoDS_Compound compound;
		BRep_Builder compoundBuilder;
		compoundBuilder.MakeCompound(compound);
		TopExp_Explorer explorer;
		for (explorer.Init(shape, TopAbs_FACE); explorer.More(); explorer.Next())
		{
			TopoDS_Face face = TopoDS::Face(explorer.Current());
			TopTools_IndexedMapOfShape wireMap;
			TopExp::MapShapes(face, TopAbs_WIRE, wireMap);
			int numWires = wireMap.Extent();
			if (wireMap.Extent() != 1)
			{
                LOG(ERROR) << "invalid number of wires found in face";
                throw CTiglError("ERROR: invalid number of wires found in face");
			}
			TopoDS_Shape s = fillWireWithNurbsSurface(TopoDS::Wire(wireMap.FindKey(1)));
			compoundBuilder.Add(compound, s);
		}
		return compound;
	}

	// helper for checking for looping edge (start and end vertex are identical)
	bool CTiglCommon::isLoopingEdge(const TopoDS_Edge& edge)
	{
		TopoDS_Vertex vertex1 = TopExp::FirstVertex(edge);
		TopoDS_Vertex vertex2 = TopExp::LastVertex(edge);
		gp_Pnt point1 = BRep_Tool::Pnt(vertex1);
		gp_Pnt point2 = BRep_Tool::Pnt(vertex2);
		return (point1.Distance(point2) < Precision::Confusion());
	}

    // helper for splitting a shape by another shape
    TopoDS_Shape CTiglCommon::splitShape(const TopoDS_Shape& src, const TopoDS_Shape& tool)
    {
        GEOMAlgo_Splitter splitter;
        // TODO: disabling parallel execution, evaluate first whehter this returns correct results!!!
        //splitter.SetRunParallel(true);
        splitter.AddArgument(src);
        splitter.AddTool(tool);
        try 
        {
            splitter.Perform();
        }
        catch (const Standard_Failure& f)
        {
            std::stringstream ss;
            ss << "ERROR: splitting of shapes failed: " << f.GetMessageString();
            LOG(ERROR) << ss.str();
            throw CTiglError(ss.str());
        }
        if (splitter.ErrorStatus() != 0)
        {
            LOG(ERROR) << "unable to split passed shapes!";
            throw CTiglError("ERROR: unable to split passed shapes!");
        }
        return splitter.Shape();
    }

	// helper for finding the face which has the lowest distance to the passed point
	TopoDS_Face CTiglCommon::getNearestFace(const TopoDS_Shape& shape, const gp_Pnt& pnt)
	{
		TopExp_Explorer explorer;
		TopoDS_Vertex v = BRepBuilderAPI_MakeVertex(pnt);
		TopoDS_Face resultFace;
		float resultDistance = std::numeric_limits<float>::max();
		for (explorer.Init(shape, TopAbs_FACE); explorer.More(); explorer.Next())
		{
			TopoDS_Face checkFace = TopoDS::Face(explorer.Current());
			BRepExtrema_DistShapeShape extrema(checkFace, v);
			if (!extrema.IsDone() || extrema.NbSolution() < 1)
			{
                LOG(ERROR) << "unable to determine nearest point between face and vertex!";
                throw CTiglError("ERROR: unable to determine nearest point between face and vertex!");
			}
			if (extrema.Value() < resultDistance)
			{
				resultFace = checkFace;
				resultDistance = extrema.Value();
			}
		}
		return resultFace;
	}

	bool CTiglCommon::checkSamePosition(const TopoDS_Vertex& v1, const TopoDS_Vertex& v2, double precision)
	{
		gp_Pnt p1 = BRep_Tool().Pnt(v1);
		gp_Pnt p2 = BRep_Tool().Pnt(v2);
		return p1.IsEqual(p2, precision);
	}

	bool CTiglCommon::checkConnectedShapes(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double precision)
	{
		BRepExtrema_DistShapeShape extrema(shape1, shape2);
		if (!extrema.IsDone())
		{
            LOG(ERROR) << "unable to determine whether two shapes are connected!";
            throw CTiglError("ERROR: unable to determine whether two shapes are connected!");
		}
		return (extrema.Value() <= precision);
	}

    bool CTiglCommon::isInside(const TopoDS_Shape& shape, const Bnd_Box& bbox)
    {
        TopTools_ListOfShape vertexList;
        TopTools_ListIteratorOfListOfShape it;
        getListOfShape(shape, TopAbs_VERTEX, vertexList);
        for (it.Initialize(vertexList); it.More(); it.Next())
        {
            TopoDS_Vertex& v = static_cast<TopoDS_Vertex&>(it.Value());
            gp_Pnt p = BRep_Tool::Pnt(v);
            if (bbox.IsOut(p))
            {
                return false;
            }
        }
        return true;
    }

    bool CTiglCommon::getMinMaxPoint(const TopoDS_Shape& shape, const gp_Vec& dir, gp_Pnt& minPnt, gp_Pnt& maxPnt)
    {
        TopExp_Explorer explorer(shape, TopAbs_VERTEX);
        
        // return false in case shape has no vertices
        if (!explorer.More())
        {
            return false;
        }

        // initialize values
        const TopoDS_Vertex& v = TopoDS::Vertex(explorer.Current());
        gp_Pnt p = BRep_Tool::Pnt(v);
        minPnt = p;
        maxPnt = p;
        explorer.Next();

        for (; explorer.More(); explorer.Next())
        {
            const TopoDS_Vertex& v = TopoDS::Vertex(explorer.Current());
            p = BRep_Tool::Pnt(v);
            gp_Vec offset(minPnt, p);
            if (offset.Dot(dir) < 0)
            {
                minPnt = p;
            }
            offset = gp_Vec(p, maxPnt);
            if (offset.Dot(dir) < 0)
            {
                maxPnt = p;
            }
        }

        return true;
    }

    TopoDS_Face CTiglCommon::buildRuledFace(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2) {
        // fix edge order in wires so that no self-intersecting faces are generated
        TopTools_SequenceOfShape wireSequence;
        wireSequence.Append(wire1);
        wireSequence.Append(wire2);
        
        BRepFill_CompatibleWires orderWires(wireSequence);
        orderWires.Perform();
        if (! orderWires.IsDone()) {
            LOG(ERROR) << "unable to determine common wire order!";
            throw CTiglError("Error: unable to determine common wire order!");
        }
        TopTools_SequenceOfShape orderedWireSequence;

        orderedWireSequence = orderWires.Shape();
        if (orderedWireSequence.Length() != 2) {
            LOG(ERROR) << "number of wires in sequence does not match. Expected: 2, Got: " << orderedWireSequence.Length();
            throw CTiglError("Error: number of wires in sequence does not match. Expected: 2");
        }
        TopoDS_Wire sortedWire1 = TopoDS::Wire(orderedWireSequence.First());
        TopoDS_Wire sortedWire2 = TopoDS::Wire(orderedWireSequence.Last());

        // build curve adaptor, second parameter defines that the length of the single edges is used as u coordinate, 
        // instead normalization of the u coordinates of the single edges of the wire (each edge would have u-coords
        // range from 0 to 1 independent of their length otherwise)
        BRepAdaptor_CompCurve compCurve1(sortedWire1, Standard_True);
        BRepAdaptor_CompCurve compCurve2(sortedWire2, Standard_True);

        // Wrap the adaptor in a class which manages curve access via handle (HCurve). According to doxygen
        // this is required by the algorithms
        Handle(Adaptor3d_HCurve) curve1 = new BRepAdaptor_HCompCurve(compCurve1);
        Handle(Adaptor3d_HCurve) curve2 = new BRepAdaptor_HCompCurve(compCurve2);

        // We have to generate an approximated curve now from the wire using the adaptor
        // NOTE: last parameter value unknown
        Approx_Curve3d approx1(curve1, Precision::Confusion(), GeomAbs_C2, 2000, 12);
        Approx_Curve3d approx2(curve2, Precision::Confusion(), GeomAbs_C2, 2000, 12);

        // Get approximated curve
        if (!approx1.IsDone() || !approx1.HasResult() ||
            !approx2.IsDone() || !approx2.HasResult()) {
            LOG(ERROR) << "unable to approximate wires by curves for building face";
            throw CTiglError("Error: unable to approximate wires by curves for building face!");
        }
        Handle(Geom_Curve) approxCurve1 = approx1.Curve();
        Handle(Geom_Curve) approxCurve2 = approx2.Curve();

        TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(approxCurve1);
        TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(approxCurve2);
        TopoDS_Face face = BRepFill::Face(e1, e2);
        return face;
    }

    TopoDS_Face CTiglCommon::getSingleFace(const TopoDS_Shape& shape)
    {
        TopTools_ListOfShape faceList;
        getListOfShape(shape, TopAbs_FACE, faceList);
        if (faceList.Extent() < 1) {
            LOG(ERROR) << "unable to get single face from shape: shape contains no faces";
            throw CTiglError("ERROR: unable to get single face from shape: shape contains no faces!");
        } else if (faceList.Extent() > 1) {
            LOG(ERROR) << "unable to get single face from shape: shape contains more than one face";
            throw CTiglError("ERROR: unable to get single face from shape: shape contains more than one face!");
        } else {
            return TopoDS::Face(faceList.First());
        }
    }
    
    gp_Pnt CTiglCommon::getMidpoint(gp_Pnt p1, gp_Pnt p2)
    {
        return gp_Pnt((p1.X() + p2.X()) / 2., (p1.Y() + p2.Y()) / 2., (p1.Z() + p2.Z()) / 2.);
    }


    TopoDS_Edge CTiglCommon::getEdgeSegment(const TopoDS_Edge& ref, const TopoDS_Vertex& v1, const TopoDS_Vertex& v2)
    {
        Standard_Real umin, umax;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(ref, umin, umax);
        BRepBuilderAPI_MakeEdge me(curve, v1, v2);
        if (me.IsDone()) {
            return me.Edge();
        }
        // otherwise try different approach
        ShapeAnalysis_Curve sac;
        Standard_Real param1, param2;
        gp_Pnt projPnt;
        gp_Pnt pnt1 = BRep_Tool::Pnt(v1);
        gp_Pnt pnt2 = BRep_Tool::Pnt(v2);
        Standard_Real offset1 = sac.Project(curve, pnt1, Precision::Confusion(), projPnt, param1);
        Standard_Real offset2 = sac.Project(curve, pnt2, Precision::Confusion(), projPnt, param2);
        if (offset1 > Precision::Confusion() || offset2 > Precision::Confusion()) {
            std::stringstream ss;
            ss << "Error getting edge segment based on reference edge and two vertices: minimum distance = " << std::max(offset1, offset2);
            LOG(ERROR) << ss.str();
            throw CTiglError(ss.str());
        }
        BRepBuilderAPI_MakeEdge me2(curve, param1, param2);
        if (!me2.IsDone()) {
            LOG(ERROR) << "Error getting edge segment based on reference edge and two vertices: OCC builder failed!";
            throw CTiglError("Error getting edge segment based on reference edge and two vertices: OCC builder failed!");
        }
        return me2.Edge();
    }

#ifdef VALIDATE_CUT_SHAPES
    void CTiglCommon::validateCutShapes(const TopoDS_Shape& s1, const TopoDS_Shape& s2)
    {
        BOPAlgo_ArgumentAnalyzer aChecker;
        aChecker.SetFuzzyValue(Precision::Confusion());
        aChecker.SetShape1(s1);
        aChecker.SetShape2(s2);
        aChecker.StopOnFirstFaulty() = Standard_False;
        aChecker.ArgumentTypeMode() = Standard_False;
        aChecker.ContinuityMode() = Standard_False;
        aChecker.CurveOnSurfaceMode() = Standard_False;
        aChecker.SelfInterMode() = Standard_True;
        aChecker.SmallEdgeMode() = Standard_True;
        aChecker.RebuildFaceMode() = Standard_True;
        aChecker.TangentMode() = Standard_True;
        aChecker.MergeVertexMode() = Standard_True;
        aChecker.MergeEdgeMode() = Standard_True;
        aChecker.OperationType() = BOPAlgo_SECTION;

        aChecker.Perform();

        if (aChecker.HasFaulty()) {
            std::stringstream ss;
            ss << "ERROR during validation of input shapes for cutting operation: ";
            BOPAlgo_ListIteratorOfListOfCheckResult anIt(aChecker.GetCheckResult());
            for (; anIt.More(); anIt.Next()) {
                const BOPAlgo_CheckResult& aResult = anIt.Value();
                const TopoDS_Shape & aSS1 = aResult.GetShape1();
                const TopoDS_Shape & aSS2 = aResult.GetShape2();
                const BOPCol_ListOfShape & aLS1 = aResult.GetFaultyShapes1();
                const BOPCol_ListOfShape & aLS2 = aResult.GetFaultyShapes2();
                Standard_Boolean isL1 = !aLS1.IsEmpty();
                Standard_Boolean isL2 = !aLS2.IsEmpty();

                switch (aResult.GetCheckStatus()) {
                case BOPAlgo_SelfIntersect: {
                    if (!aSS1.IsNull()) {
                        ss << "First shape has self intersections. ";
                    }
                    if (!aSS2.IsNull()) {
                        ss << "Second shape has self intersections. ";
                    }
                }
                    break;
                case BOPAlgo_TooSmallEdge: {
                    if (!aSS1.IsNull()) {
                        ss << "First shape has too small edges. ";
                    }
                    if (!aSS2.IsNull()) {
                        ss << "Second shape has too small edges. ";
                    }
                }
                    break;
                case BOPAlgo_NonRecoverableFace: {
                    if (!aSS1.IsNull()) {
                        ss << "First shape has bad faces. ";
                    }
                    if (!aSS2.IsNull()) {
                        ss << "Second shape has bad faces. ";
                    }
                }
                    break;
                case BOPAlgo_IncompatibilityOfVertex: {
                    if (!aSS1.IsNull()) {
                        ss << "First shape has too close vertices. ";
                    }
                    if (!aSS2.IsNull()) {
                        ss << "Second shape has too close vertices. ";
                    }
                }
                    break;
                case BOPAlgo_IncompatibilityOfEdge: {
                    if (!aSS1.IsNull()) {
                        ss << "First shape has too close edges. ";
                    }
                    if (!aSS2.IsNull()) {
                        ss << "Second shape has too close edges. ";
                    }
                }
                    break;
                case BOPAlgo_IncompatibilityOfFace: {
                    if (!aSS1.IsNull()) {
                        ss << "First shape has too close faces. ";
                    }
                    if (!aSS2.IsNull()) {
                        ss << "Second shape has too close faces. ";
                    }
                }
                    break;
                case BOPAlgo_InvalidCurveOnSurface: {
                    if (!aSS1.IsNull()) {
                        ss << "First shape has invalid curves on surface. ";
                    }
                    if (!aSS2.IsNull()) {
                        ss << "Second shape has invalid curves on surface. ";
                    }
                }
                    break;
                default:
                    if (!aSS1.IsNull()) {
                        ss << "First shape has unknown failure. ";
                    }
                    if (!aSS2.IsNull()) {
                        ss << "Second shape has unknown failure. ";
                    }
                }

                static int counter = 0;
                dumpShape(s1, VALIDATE_CUT_SHAPE_DIRECTORY, "cutShape1_", counter);
                dumpShape(s2, VALIDATE_CUT_SHAPE_DIRECTORY, "cutShape2_", counter);
                ss << "Checked shapes written to directory " << VALIDATE_CUT_SHAPE_DIRECTORY << " and file names cutShape1_" << counter << ".brep and cutShape2_" << counter << ".brep";
                counter++;

                std::cerr << ss.str();
                LOG(ERROR) << ss.str();
                throw CTiglError(ss.str());
            }
        }
    }
#endif

} // namespace tigl
