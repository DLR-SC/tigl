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
#ifndef CTIGLCOMMON_H
#define CTIGLCOMMON_H

#include <Precision.hxx>
#include <TopAbs_ShapeEnum.hxx>
// included for enum type 'TiglSymmetryAxis'
#include "tigl.h"

// forward declarations
class gp_Pnt;
class gp_Vec;
class Bnd_Box;
class Handle_Geom_Curve;
class Handle_Geom_Surface;
class TopoDS_Compound;
class TopoDS_Shape;
class TopoDS_Edge;
class TopoDS_Face;
class TopoDS_Shell;
class TopoDS_Vertex;
class TopoDS_Wire;
class TopTools_ListOfShape;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


namespace tigl {


	class CTiglCommon
	{
	public:
		// Method for finding all directly and indirectly connected edges
		static void findAllConnectedEdges(const TopoDS_Edge& edge, TopTools_ListOfShape& edgeList, TopTools_ListOfShape& targetList);

		// Method for building a face out of 4 points
		static TopoDS_Face buildFace(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3, const gp_Pnt& p4);

		// Method for building a face out of two wires
		static TopoDS_Face buildFace(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2);

		// Method for building a face out of two wires and a direction vector
		static TopoDS_Face buildFace(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2, const gp_Vec& dir);

        // Method for building a face out of closed wire.
        // The algorithm tries to build a face using a plane as surface, in case this fails
        // the BRepFill_Filling class is used.
        static TopoDS_Face buildFace(const TopoDS_Wire& wire);

		// Method for building a wire out of two points
		static TopoDS_Wire buildWire(const gp_Pnt& p1, const gp_Pnt& p2);

		// Method for cutting two shapes, resulting in the common geometry (e.g. intersection edges)
		static TopoDS_Shape cutShapes(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, bool cutSingleFaces=false);

        // Method tries cutting the shapes with different options until a result of the expected type was found
        // If no result can be found an exception is thrown
        static TopoDS_Shape cutShapesWithExpectedResult(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, 
            TopAbs_ShapeEnum expectedResult, bool cutSingleFaces=false);

        // Method for finding the intersection point of a curve and a surface
    static bool getIntersectionPoint(Handle_Geom_Surface surface, Handle_Geom_Curve curve, gp_Pnt& dst);

		// Method for finding the intersection point of a face and an edge
		static bool getIntersectionPoint(const TopoDS_Face& face, const TopoDS_Edge& edge, gp_Pnt& dst);

		// Method for finding the intersection point of a face and a wire (containing edges)
		static bool getIntersectionPoint(const TopoDS_Face& face, const TopoDS_Wire& wire, gp_Pnt& dst);

		// Method for getting a list of subshapes of a passed geometry
		static void getListOfShape(const TopoDS_Shape& shape, TopAbs_ShapeEnum type, TopTools_ListOfShape& result);

		// Method for building a wire out of the edges from the passed geometry
		static TopoDS_Wire makeWireFromEdges(const TopoDS_Shape& edges);

		// Method for getting all wires from the passed shape (list of edges)
		static void makeWiresFromConnectedEdges(const TopoDS_Shape& shape, TopTools_ListOfShape& wireList);

        // Method for sorting all edges in a shape
        // This uses makeWiresFromConnectedEdges in order to separate all edges 
        // of the shape into wires with sorted edges and then combines it again
        // to a compound
        static TopoDS_Shape makeSortedEdgeShape(const TopoDS_Shape& shape);

		// Method for creating a face from an opened wire
		static TopoDS_Wire closeWire(const TopoDS_Wire& wire);

		// Method for closing two wires to a single one, 
		// The method determines a direction vector based on the end vertices of wire1
		// and calls the second closeWires method
		static TopoDS_Wire closeWires(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2);

		// Method for closing two wires to a single one, 
		// the passed vector is used to define the upper and lower end vertices of the wires
		static TopoDS_Wire closeWires(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2, const gp_Vec& dir);

		// Method for searching all vertices which are only connected to a single edge
		static void getEndVertices(const TopoDS_Shape& shape, TopTools_ListOfShape& endVertices);

		// Method for sorting the edges of a wire
		static TopoDS_Wire sortWireEdges(const TopoDS_Wire& wire, bool closed);

		// Method for checking whether two edges have a common vertex (regarding the position)
		static bool checkCommonVertex(const TopoDS_Edge& e1, const TopoDS_Edge& e2);

		// Method for removing duplicate edges
		static TopoDS_Shape removeDuplicateEdges(const TopoDS_Shape& shape);

		// Dump brep-representation of shape to file
		static void dumpShape(const TopoDS_Shape& shape, std::string prefix, std::string fileName, int counter = -1);

		// Helper Function for limiting double to range.
		// Throws exception when difference to min/max value is larger then Precision
		static double limitRange(double d, double min, double max);

		// Compute the angle between two edges
		static double getAngleBetweenEdges(const TopoDS_Edge& edge1, const TopoDS_Edge& edge2);

		// Split up the passed closed wire into a list of 4 wires, representing the four edges of a face
		// NOTE: This method requires a closed wire to be passed
		static void splitFaceWires(const TopoDS_Wire& wire, TopTools_ListOfShape& wireList);

		// Fills the wire with a BSplineSurface, so that the UV-coordinates of the surface
		// match to the area inside the face
		static TopoDS_Face fillWireWithNurbsSurface(const TopoDS_Wire& wire);

		// Generates a face based on a BSplineSurface where the wires in the passed list are the boundaries
		static TopoDS_Face fillWireListWithNurbsSurface(const TopTools_ListOfShape& wireList);

        // moved from CTIGLViewerDocument
        // [[CAS_AES]] added method for mirroring geometry
        static gp_Pnt mirrorPoint(gp_Pnt pnt, TiglSymmetryAxis symmetryAxis);
        
        // moved from CTIGLViewerDocument
        // [[CAS_AES]] added method for mirroring geometry
        static TopoDS_Shape mirrorShape(const TopoDS_Shape& shape, TiglSymmetryAxis symmetryAxis, bool retBoth = true);

        // method for mirroring shape via a plane normal to the passed direction
        static TopoDS_Compound mirrorShape(const TopoDS_Shape& shape, gp_Vec direction, bool retBoth = true);

        // cutting operation with the special purpose to cut a wing with a fuselage (or an other wing)
        // The first shape is every face, which is outside of the bounding box of the cutting shape
        // The second shape will also be cutted with the first shape and the ouside surfaces will be returned
        static void trimWing( TopoDS_Shape&, TopoDS_Shape&);


		// iterate over all faces and generate NURBS surfaces
		static TopoDS_Shape buildNurbsFaces(const TopoDS_Shape& shape);

		// helper for checking for looping edge (start and end vertex are identical)
		static bool isLoopingEdge(const TopoDS_Edge& edge);

		// helper for splitting a shape by another shape
		static TopoDS_Shape splitShape(const TopoDS_Shape& src, const TopoDS_Shape& tool);

		// helper for finding the face which has the lowest distance to the passed point
		static TopoDS_Face getNearestFace(const TopoDS_Shape& src, const gp_Pnt& pnt);

		// helper for checking whether two vertices are located at the same position
		static bool checkSamePosition(const TopoDS_Vertex& v1, const TopoDS_Vertex& v2, double precision = Precision::Confusion());

		// helper for checking whether two shapes are connected (minimum distance < precision)
		static bool checkConnectedShapes(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double precision = Precision::Confusion());

        // helper for checking if all vertices of a shape lie within a bounding box
        static bool isInside(const TopoDS_Shape& shape, const Bnd_Box& bbox);

        // helper for getting the vertex positions with the minimum and maximum
        // extent in a defined direction from a passed shape
        // Returns true when the minimum/maximum points could be found
        static bool getMinMaxPoint(const TopoDS_Shape& shape, const gp_Vec& dir, gp_Pnt& minPnt, gp_Pnt& maxPnt);

        // Method for building a ruled face between the two wires. The method
        // approximates the two wires by curves, and generates a single face
        // between these curves.
        // This can be used for generating non-planar faces. For planar faces
        // see buildFace method
        static TopoDS_Face buildRuledFace(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2);

        // helper for getting face of shape which consist only of a single face
        static TopoDS_Face getSingleFace(const TopoDS_Shape& shape);
        
        // returns a Point exactly in the middle of the 2 input points
        static gp_Pnt getMidpoint(gp_Pnt, gp_Pnt);

        // Method for getting the segment of the edge which lies beside the two passed vertices
        // This method tries to compute the segment on two ways, becase in OCC 6.9 the 
        // BRepBuilder_API_MakeEdge class fails in some cases
        static TopoDS_Edge getEdgeSegment(const TopoDS_Edge& ref, const TopoDS_Vertex& v1, const TopoDS_Vertex& v2);

    private:
        static TopoDS_Shape cutShapesInternal(const TopoDS_Shape& s1, const TopoDS_Shape& s2, bool cutSingleFaces,
            bool approx, bool usepcurve, bool invert);

#ifdef VALIDATE_CUT_SHAPES
        static void validateCutShapes(const TopoDS_Shape& s1, const TopoDS_Shape& s2);
#endif


	}; // CTiglCommon

} // namespace tigl

#endif // CTIGLCOMMON_H
