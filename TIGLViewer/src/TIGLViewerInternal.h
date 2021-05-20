/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerInternal.h 226 2012-10-23 19:18:29Z martinsiggel $
*
* Version: $Revision: 226 $
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

#ifndef TIGLVIEWERINTERNAL_H
#define TIGLVIEWERINTERNAL_H

#include <limits.h>

#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <AIS_ListOfInteractive.hxx>
#include <AIS_ListIteratorOfListOfInteractive.hxx>
#include <AIS_Shape.hxx>
#include <AIS_SequenceOfInteractive.hxx>
#include <AIS_Trihedron.hxx>

#include <Aspect_Background.hxx>
#include <Aspect_GridDrawMode.hxx>
#include <Aspect_GridType.hxx>
#include <Aspect_RectangularGrid.hxx>
#include <Aspect_TypeOfLine.hxx>
#include <Aspect_WidthOfLine.hxx>
#include <Aspect_Window.hxx>

#include <Bnd_Box2d.hxx>
#include <BndLib_Add2dCurve.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>

#include <BRepBuilderAPI.hxx>
#include <BRepAlgo.hxx>

#include <BRepTools.hxx>
#include <Standard_DefineHandle.hxx>
#include <DsgPrs_LengthPresentation.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <Geom_Axis2Placement.hxx>
#include <Geom_CartesianPoint.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>
#include <Geom2d_BezierCurve.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <GeomAbs_CurveType.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomTools_Curve2dSet.hxx>

#include <IntAna_IntConicQuad.hxx>

#include <gp_Ax2d.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec.hxx>
#include <gp_Vec2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pln.hxx>
#include <gp_Lin.hxx>
#include <gp_Dir.hxx>

#include <Graphic3d_AspectMarker3d.hxx>
#include <Graphic3d_NameOfMaterial.hxx>
#include <MMgt_TShared.hxx>
#include <OSD_Environment.hxx>
#include <Precision.hxx>
#include <Prs3d_IsoAspect.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_Text.hxx>
#include <Quantity_NameOfColor.hxx>
#include <Quantity_PhysicalQuantity.hxx>
#include <Quantity_TypeOfColor.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <SelectMgr_SelectableObject.hxx>
#include <SelectMgr_Selection.hxx>
#include <SelectMgr_SelectionManager.hxx>
#include <Standard_Boolean.hxx>
#include <Standard_CString.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_IStream.hxx>
#include <Standard_Macro.hxx>
#include <Standard_NotImplemented.hxx>
#include <Standard_OStream.hxx>
#include <Standard_Real.hxx>
#include <StdPrs_Curve.hxx>
#include <StdPrs_Point.hxx>
#include <StdPrs_PoleCurve.hxx>
#include <TCollection_AsciiString.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <TColgp_HArray1OfPnt2d.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TColStd_HSequenceOfTransient.hxx>
#include <TColStd_MapIteratorOfMapOfTransient.hxx>
#include <TColStd_MapOfTransient.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_ListIteratorOfListOfShape.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopExp.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <UnitsAPI.hxx>
#include <V3d_RectangularGrid.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

// specific to ISession2D_Shape
#include <TopTools_ListOfShape.hxx>
#include <HLRAlgo_Projector.hxx>
#include <HLRBRep_Algo.hxx>
#include <HLRBRep_PolyAlgo.hxx>
#include <HLRBRep_PolyHLRToShape.hxx>
#include <HLRBRep_HLRToShape.hxx>
#include <HLRAlgo_Projector.hxx>

// specific IGES, STEP or DXF
#include <XSControl_WorkSession.hxx>
#include <STEPControl_StepModelType.hxx>
#include <IFSelect_ReturnStatus.hxx>

// specific IGES STEP
// #include <Interface_DT.hxx>
// specific IGES, STEP or DXF
#include <Interface_Static.hxx>

// specific STEP
#include <STEPControl_StepModelType.hxx>
#include <STEPControl_Controller.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>

// specific IGES
#include <Interface_InterfaceModel.hxx>
#include <IGESControl_Controller.hxx>
#include <IGESControl_Writer.hxx>
#include <IGESControl_Reader.hxx>
#include <IGESToBRep_Actor.hxx>
#include <IGESToBRep_Reader.hxx>

// specific STL
#include <StlAPI_Writer.hxx>
#include <StlAPI_Reader.hxx>

// specific VRML
#include <VrmlAPI_Writer.hxx>

// specific MESH
#include "CHotsoseMeshReader.h"

// The OpenCASCADE includes...
#include <AIS_InteractiveContext.hxx>

#endif // TIGLVIEWERINTERNAL_H

