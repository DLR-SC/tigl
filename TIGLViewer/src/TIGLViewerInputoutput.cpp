/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerInputoutput.cpp 212 2012-10-15 15:33:53Z martinsiggel $
*
* Version: $Revision: 212 $
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

#include <QApplication>

#include "TIGLViewerInternal.h"
#include "TIGLViewerInputoutput.h"
#include "TIGLViewerWidget.h"
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <CTiglPolyDataTools.h>
#include <TIGLAISTriangulation.h>
#include <TColStd_HArray1OfInteger.hxx>
#include <Poly_Triangulation.hxx>
#include <IGESData_IGESModel.hxx>
#include <AIS_ListIteratorOfListOfInteractive.hxx>
#include <AIS_Shape.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Line.hxx>

bool TIGLViewerInputOutput::importModel( const QString& fileName,
                                         const FileFormat format,
                                         TIGLViewerContext& scene )
{

    QApplication::setOverrideCursor( Qt::WaitCursor );
    scene.getContext()->SetDisplayMode(AIS_Shaded,Standard_False);
    Handle(TopTools_HSequenceOfShape) shapes = importModel( format, fileName );
    QApplication::restoreOverrideCursor();

    if ( shapes.IsNull() || !shapes->Length() ) {
        return false;
    }

    for ( int i = 1; i <= shapes->Length(); i++ ) {
        scene.displayShape(shapes->Value(i), true);
    }
    return true;
}

bool TIGLViewerInputOutput::importTriangulation( const QString& fileName,
                                                 const FileFormat format,
                                                 TIGLViewerContext& scene )
{
    Handle(AIS_InteractiveContext) ic = scene.getContext();
    Handle(Poly_Triangulation) triangulation;
    triangulation.Nullify();

    if (format == FormatMESH) {
        CHotsoseMeshReader meshReader;
        tigl::CTiglPolyData mesh;
        if (meshReader.readFromFile(fileName.toStdString().c_str(), mesh) != TIGL_SUCCESS){
            return false;
        }
        triangulation = tigl::CTiglPolyDataTools::MakePoly_Triangulation(mesh);
    }
    else {
        return false;
    }
    Handle(TIGLAISTriangulation) shape = new TIGLAISTriangulation(triangulation);
    shape->SetMaterial(Graphic3d_NOM_METALIZED);
    // alpha , blue, green, red
    Standard_Integer color = (0 << 24)
                           + (20 << 16)
                           + (20 << 8)
                           +  20;
    Handle(TColStd_HArray1OfInteger) colors = new TColStd_HArray1OfInteger(1,triangulation->NbNodes());
    for (Standard_Integer i = colors->Lower(); i <= colors->Upper(); ++i) {
        colors->SetValue(i, color);
    }
    shape->SetColors(colors);
    shape->SetDisplayMode(AIS_Shaded);
    ic->Display(shape,Standard_False);
    ic->UpdateCurrentViewer();

    return true;
}
Handle(TopTools_HSequenceOfShape) TIGLViewerInputOutput::importModel( const FileFormat format, const QString& file )
{
    Handle(TopTools_HSequenceOfShape) shapes;
    try {
        switch ( format ) {
        case FormatBREP:
            shapes = importBREP( file );
            break;
        case FormatIGES:
            shapes = importIGES( file );
            break;
        case FormatSTL:
            shapes = importSTL ( file );
            break;
        case FormatSTEP:
            shapes = importSTEP( file );
            break;
        case FormatMESH:
            shapes = importMESH( file );
            break;
        default:
            // To Do - Error message here?
            break;
        }
    } catch ( Standard_Failure ) {
        shapes.Nullify();
    }
    return shapes;
}


/******************************************************************
 * EXPORT FUNCTIONALITY
 ******************************************************************/

bool TIGLViewerInputOutput::exportModel( const QString& fileName,
                                         const FileFormat format,
                                         const Handle(AIS_InteractiveContext)& ic )
{

    Handle(TopTools_HSequenceOfShape) shapes = getShapes( ic );
    if ( shapes.IsNull() || !shapes->Length() ) {
        return false;
    }

    QApplication::setOverrideCursor( Qt::WaitCursor );
    bool stat = exportModel( format, fileName, shapes );
    QApplication::restoreOverrideCursor();
    return stat;
}

bool TIGLViewerInputOutput::exportModel( const FileFormat format, const QString& file, const Handle(TopTools_HSequenceOfShape)& shapes )
{
    bool status = false;
    try {
        switch ( format ) {
        case FormatBREP:
            status = exportBREP( file, shapes );
            break;
        case FormatIGES:
            status = exportIGES( file, shapes );
            break;
        case FormatSTEP:
            status = exportSTEP( file, shapes );
            break;
        case FormatSTL:
            status = exportSTL( file, shapes );
            break;
        default:
            status = false;
            break;
        }
    } catch ( Standard_Failure ) {
        status = false;
    }
    return status;
}

Handle(TopTools_HSequenceOfShape) TIGLViewerInputOutput::getShapes( const Handle(AIS_InteractiveContext)& ic )
{
    Handle(TopTools_HSequenceOfShape) aSequence;
    Handle(AIS_InteractiveObject) picked;
    // export selected objects
    for ( ic->InitSelected(); ic->MoreSelected(); ic->NextSelected() ) {
        Handle(AIS_InteractiveObject) obj = ic->SelectedInteractive();
        if ( obj->IsKind( STANDARD_TYPE( AIS_Shape ) ) ) {
            TopoDS_Shape shape = Handle(AIS_Shape)::DownCast(obj)->Shape();
            if ( aSequence.IsNull() ) {
                aSequence = new TopTools_HSequenceOfShape();
            }
            aSequence->Append( shape );
        }
    }
    if (!aSequence.IsNull() && aSequence->Length() > 0) {
        return aSequence;
    }

    // if no objects are selected, export all objects
    AIS_ListOfInteractive listAISShapes;
    ic->DisplayedObjects(listAISShapes);
    AIS_ListIteratorOfListOfInteractive iter(listAISShapes);
    for (; iter.More(); iter.Next()) {
        Handle(AIS_InteractiveObject) obj = iter.Value();
        if ( obj->IsKind( STANDARD_TYPE( AIS_Shape ) ) ) {
            TopoDS_Shape shape = Handle(AIS_Shape)::DownCast(obj)->Shape();
            if ( aSequence.IsNull() ) {
                aSequence = new TopTools_HSequenceOfShape();
            }
            aSequence->Append( shape );
        }
    }
    return aSequence;
}


Handle(TopTools_HSequenceOfShape) TIGLViewerInputOutput::importBREP( const QString& file )
{
    Handle(TopTools_HSequenceOfShape) aSequence;
    TopoDS_Shape aShape;
    BRep_Builder aBuilder;

    Standard_Boolean result = BRepTools::Read(  aShape, file.toLatin1().data(), aBuilder );
    if ( result ) {
        aSequence = new TopTools_HSequenceOfShape();
        if (aShape.ShapeType() == TopAbs_COMPOUND) {
            for (TopoDS_Iterator anIter(aShape); anIter.More(); anIter.Next()) {
                const TopoDS_Shape& aSh = anIter.Value();
                aSequence->Append( aSh );
            }
        }
        else {
            aSequence->Append(aShape);
        }
    }

    return aSequence;
}

Handle(TopTools_HSequenceOfShape) TIGLViewerInputOutput::importIGES( const QString& file )
{
    Handle(TopTools_HSequenceOfShape) aSequence;
    IGESControl_Reader Reader;
    Interface_Static::SetCVal("xstep.cascade.unit", "M");
    int status = Reader.ReadFile( file.toLatin1().data() );

    if ( status == IFSelect_RetDone ) {
        Reader.TransferRoots();
        int nbs = Reader.NbShapes();
        if ( nbs > 0 ) {
            aSequence = new TopTools_HSequenceOfShape();
            for ( int i = 1; i <= nbs; i++ ) {
                TopoDS_Shape shape = Reader.Shape( i );
                aSequence->Append( shape );
            }
        }
    }
    return aSequence;
}

Handle(TopTools_HSequenceOfShape) TIGLViewerInputOutput::importSTL( const QString& file )
{
    Handle(TopTools_HSequenceOfShape) aSequence = nullptr;
    TopoDS_Shape aShape;
    StlAPI_Reader Reader;
    Reader.Read(aShape, file.toStdString().c_str());
    

    if (!aShape.IsNull()) {
        aSequence = new TopTools_HSequenceOfShape();
        aSequence->Append(aShape);
    }

    return aSequence;
}

Handle(TopTools_HSequenceOfShape) TIGLViewerInputOutput::importMESH( const QString& file )
{
    Handle(TopTools_HSequenceOfShape) aSequence = nullptr;

    CHotsoseMeshReader meshReader;
    tigl::CTiglPolyData mesh;
    if (meshReader.readFromFile(file.toStdString().c_str(), mesh) != TIGL_SUCCESS){
        return aSequence;
    }
    aSequence = new TopTools_HSequenceOfShape();

    TopoDS_Shape shape = tigl::CTiglPolyDataTools::MakeTopoDS(mesh);
    aSequence->Append(shape);

    return aSequence;
}

Handle(TopTools_HSequenceOfShape) TIGLViewerInputOutput::importSTEP( const QString& file )
{
    Handle(TopTools_HSequenceOfShape) aSequence = new TopTools_HSequenceOfShape;

    STEPControl_Reader aReader;
    Interface_Static::SetCVal("xstep.cascade.unit", "M");
    IFSelect_ReturnStatus status = aReader.ReadFile( file.toLatin1().data() );
    if ( status == IFSelect_RetDone ) {
        //Interface_TraceFile::SetDefault();
        bool failsonly = false;
        aReader.PrintCheckLoad( failsonly, IFSelect_ItemsByEntity );

        int nbr = aReader.NbRootsForTransfer();
        aReader.PrintCheckTransfer( failsonly, IFSelect_ItemsByEntity );
        for ( Standard_Integer n = 1; n <= nbr; n++ ) {
            bool ok = aReader.TransferRoot( n );
            if (!ok) {
                continue;
            }
            int nbs = aReader.NbShapes();
            if ( nbs > 0 ) {
                for ( int i = 1; i <= nbs; i++ ) {
                    TopoDS_Shape shape = aReader.Shape( i );
                    aSequence->Append( shape );
                }
            }
            aReader.ClearShapes();
        }
    }
    
    if (aSequence->Length() == 0) {
        aSequence.Nullify();
    }
    return aSequence;
}


bool TIGLViewerInputOutput::exportBREP( const QString& file, const Handle(TopTools_HSequenceOfShape)& shapes )
{
    if ( shapes.IsNull() || shapes->IsEmpty() ){
        return false;
    }

    TopoDS_Shape shape;
    if (shapes->Length() > 1) {
        TopoDS_Compound compound;
        BRep_Builder b;
        b.MakeCompound(compound);

        for (int ishape = 1; ishape <= shapes->Length(); ++ishape) {
            b.Add(compound, shapes->Value(ishape));
        }
        shape = compound;
    }
    else {
        shape = shapes->Value(1);
    }
    return BRepTools::Write( shape, file.toLatin1().data() );
}

bool TIGLViewerInputOutput::exportIGES( const QString& file, const Handle(TopTools_HSequenceOfShape)& shapes )
{
    if ( shapes.IsNull() || shapes->IsEmpty() ) {
        return false;
    }

    IGESControl_Controller::Init();
    Interface_Static::SetCVal("xstep.cascade.unit", "M");
    Interface_Static::SetCVal("write.iges.unit", "M");
    Interface_Static::SetIVal("write.iges.brep.mode", 0);
    Interface_Static::SetCVal("write.iges.header.author", "TiGL");
    Interface_Static::SetCVal("write.iges.header.company", "German Aerospace Center (DLR), SC");
    IGESControl_Writer writer;
    writer.Model()->ApplyStatic();

    for ( int i = 1; i <= shapes->Length(); i++ ) {
        writer.AddShape ( shapes->Value( i ) );
    }
    writer.ComputeModel();
    return writer.Write( file.toLatin1().data() );
}

bool TIGLViewerInputOutput::exportSTEP( const QString& file, const Handle(TopTools_HSequenceOfShape)& shapes )
{
    if ( shapes.IsNull() || shapes->IsEmpty() ) {
        return false;
    }

    IFSelect_ReturnStatus status;
    STEPControl_Controller::Init();
    Interface_Static::SetCVal("xstep.cascade.unit", "M");
    Interface_Static::SetCVal("write.step.unit", "M");
    STEPControl_Writer writer;
    for ( int i = 1; i <= shapes->Length(); i++ ) {
        status = writer.Transfer( shapes->Value( i ), STEPControl_AsIs );
        if ( status != IFSelect_RetDone ) {
            return false;
        }
    }

    status = writer.Write( file.toLatin1().data() );

    switch ( status ) {
    case IFSelect_RetError:
        myInfo = tr( "INF_DATA_ERROR" );
        break;
    case IFSelect_RetFail:
        myInfo = tr( "INF_WRITING_ERROR" );
        break;
    case IFSelect_RetVoid:
        myInfo = tr( "INF_NOTHING_ERROR" );
        break;
    default:
        break;
    }
    return status == IFSelect_RetDone;
}

bool TIGLViewerInputOutput::exportSTL( const QString& file, const Handle(TopTools_HSequenceOfShape)& shapes )
{
    if ( shapes.IsNull() || shapes->IsEmpty() ) {
        return false;
    }

    TopoDS_Compound res;
    BRep_Builder builder;
    builder.MakeCompound( res );

    for ( int i = 1; i <= shapes->Length(); i++ ) {
        TopoDS_Shape shape = shapes->Value( i );
        if ( shape.IsNull() ) {
            myInfo = tr( "INF_TRANSLATE_ERROR_INVALIDSHAPE" );
            return false;
        }
        builder.Add( res, shape );
    }

    StlAPI_Writer writer;
    writer.Write( res, file.toLatin1().data() );

    return true;
}

bool TIGLViewerInputOutput::checkFacetedBrep( const Handle(TopTools_HSequenceOfShape)& shapes )
{
    bool err = false;
    for ( int i = 1; i <= shapes->Length(); i++ ) {
        TopoDS_Shape shape = shapes->Value( i );
        for ( TopExp_Explorer fexp( shape, TopAbs_FACE ); fexp.More() && !err; fexp.Next() ) {
            Handle(Geom_Surface) surface = BRep_Tool::Surface( TopoDS::Face( fexp.Current() ) );
            if ( !surface->IsKind( STANDARD_TYPE( Geom_Plane ) ) ) {
                err = true;
            }
        }
        for ( TopExp_Explorer eexp( shape, TopAbs_EDGE ); eexp.More() && !err; eexp.Next() ) {
            Standard_Real fd, ld;
            Handle(Geom_Curve) curve = BRep_Tool::Curve( TopoDS::Edge( eexp.Current() ), fd, ld );
            if ( !curve->IsKind( STANDARD_TYPE( Geom_Line ) ) ) {
                err = true;
            }
        }
    }
    return !err;
}
