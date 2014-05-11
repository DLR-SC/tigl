
#include <STEPControl_Reader.hxx>
#include <IGESControl_Reader.hxx>
#include "CADImport.h"
#include "OsgMainApp.hpp"
#include "CNamedShape.h"

#include <Interface_Static.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Iterator.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>

#include <string>
#include <CTiglLogging.h>
#include <CCPACSConfiguration.h>
#include <CCPACSConfigurationManager.h>
#include <CCPACSFuselageSegment.h>
#include <CCPACSWingSegment.h>

void openCADModel( const FileFormat format, const std::string& file )
{
    ListPNamedShape shapes;
    try {
        switch ( format ) {
        case FormatBREP:
            shapes = importBREP( file );
            break;
        case FormatIGES:
            shapes = importIGES( file );
            break;
        case FormatSTEP:
            shapes = importSTEP( file );
            break;
        case FormatCPACS:
            shapes = importCPACS( file );
            break;
        default:
            break;
        }
    } catch ( Standard_Failure ) {
        return;
    }

    for ( int i = 0; i < shapes.size(); i++ ) {
        PNamedShape pshape = shapes[i];
        OsgMainApp::Instance().displayShape(pshape->Shape(), pshape->Name());
    }
}

ListPNamedShape importSTEP( const std::string& file )
{
    ListPNamedShape aSequence;

    STEPControl_Reader aReader;
    Interface_Static::SetCVal("xstep.cascade.unit", "M");
    IFSelect_ReturnStatus status = aReader.ReadFile( file.c_str() );
    if ( status == IFSelect_RetDone ) {
        bool failsonly = false;
        aReader.PrintCheckLoad( failsonly, IFSelect_ItemsByEntity );

        int nbr = aReader.NbRootsForTransfer();
        aReader.PrintCheckTransfer( failsonly, IFSelect_ItemsByEntity );
        static int iShape = 0;
        for ( Standard_Integer n = 1; n <= nbr; n++ ) {
            bool ok = aReader.TransferRoot( n );
            if (!ok) {
                continue;
            }
            int nbs = aReader.NbShapes();
            if ( nbs > 0 ) {
                for ( int i = 1; i <= nbs; i++ ) {
                    TopoDS_Shape shape = aReader.Shape( i );
                    std::stringstream str;
                    str << "STEP_IMPORT_" << iShape++;
                    PNamedShape pshape(new CNamedShape(shape, str.str().c_str()));
                    aSequence.push_back(pshape);
                }
            }
            aReader.ClearShapes();
        }
    }
    return aSequence;
}

ListPNamedShape importIGES( const std::string& file )
{
    ListPNamedShape aSequence;
    IGESControl_Reader Reader;
    Interface_Static::SetCVal("xstep.cascade.unit", "M");
    int status = Reader.ReadFile( file.c_str() );
    LOG(WARNING) << "1";
    static int iShape = 0;
    if ( status == IFSelect_RetDone ) {
        Reader.TransferRoots();
        int nbs = Reader.NbShapes();
        LOG(WARNING) << "2";
        if ( nbs > 0 ) {
            for ( int i = 1; i <= nbs; i++ ) {
                LOG(WARNING) << "3";
                std::stringstream str;
                str << "IGES_IMPORT_" << iShape++;
                TopoDS_Shape shape = Reader.Shape( i );
                PNamedShape pshape(new CNamedShape(shape, str.str().c_str()));
                aSequence.push_back(pshape);
            }
        }
    }
    return aSequence;
}

ListPNamedShape importBREP( const std::string& file )
{
    ListPNamedShape aSequence;
    TopoDS_Shape aShape;
    BRep_Builder aBuilder;

    Standard_Boolean result = BRepTools::Read(  aShape, file.c_str(), aBuilder );
    static int iShape = 0;
    if ( result ) {
        if (aShape.ShapeType() == TopAbs_COMPOUND) {
            for (TopoDS_Iterator anIter(aShape); anIter.More(); anIter.Next()) {
                TopoDS_Shape aSh = anIter.Value();
                std::stringstream str;
                str << "BREP_IMPORT_" << iShape++;
                aSequence.push_back(PNamedShape(new CNamedShape(aSh, str.str().c_str() )));
            }
        }
        else {
            std::stringstream str;
            str << "BREP_IMPORT_" << iShape++;
            aSequence.push_back(PNamedShape(new CNamedShape(aShape, str.str().c_str() )));
        }
    }

    return aSequence;
}

ListPNamedShape importCPACS(const std::string& filepath)
{
    ListPNamedShape aSequence;
    TixiDocumentHandle handle = -1;
    TiglCPACSConfigurationHandle tiglHandle = -1;

    if (tixiOpenDocument(filepath.c_str(), &handle) != SUCCESS) {
        return aSequence;
    }

    if (tiglOpenCPACSConfiguration(handle, "", &tiglHandle) != TIGL_SUCCESS) {
        LOG(ERROR) << "Error opening cpacs file " << filepath;
    }

    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);

    for (int iWing = 1; iWing <= config.GetWingCount(); ++iWing) {
        tigl::CCPACSWing& wing = config.GetWing(iWing);

        for (int iSegment = 1; iSegment <= wing.GetSegmentCount(); ++iSegment) {
            tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(iSegment);
            PNamedShape pshape(new CNamedShape(segment.GetLoft(), segment.GetUID().c_str()));
            aSequence.push_back(pshape);
        }

        if (wing.GetSymmetryAxis() == TIGL_NO_SYMMETRY) {
            continue;
        }

        for (int i = 1; i <= wing.GetSegmentCount(); i++) {
            tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
            TopoDS_Shape loft = segment.GetMirroredLoft();
            PNamedShape pshape(new CNamedShape(loft, std::string(segment.GetUID() + "_mirrored").c_str()));
            aSequence.push_back(pshape);
        }

    }

    for (int f = 1; f <= config.GetFuselageCount(); f++) {
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(f);

        for (int i = 1; i <= fuselage.GetSegmentCount(); i++) {
            tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
            TopoDS_Shape loft = segment.GetLoft();
            PNamedShape pshape(new CNamedShape(loft, segment.GetUID().c_str()));
            aSequence.push_back(pshape);
        }
    }
    return aSequence;
}
