/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
/**
* @file
* @brief  Export routines for CPACS configurations.
*/

// standard libraries
#include <iostream>
#include <algorithm>
#include <ostream>

#include "CTiglLogging.h"
#include "CTiglExportVtk.h"
#include "CCPACSConfiguration.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "CTiglAbstractSegment.h"
#include "CCPACSWing.h"
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSWingSegment.h"
#include "CNamedShape.h"
#include "CGroupShapes.h"
#include "CTiglExporterFactory.h"
#include "CTiglTypeRegistry.h"

// algorithms
// #include "BRepMesh.hxx"


#include "CTiglPolyData.h"
#include "CTiglTriangularizer.h"
#include "CTiglPolyData.h"

namespace
{
    void setMinMax(const tigl::CTiglPoint& p, double* oldmin, double* oldmax)
    {
        if (p.x > *oldmax) {
            *oldmax = p.x;
        }
    
        if (p.y > *oldmax) {
            *oldmax = p.y;
        }
    
        if (p.z > *oldmax) {
            *oldmax = p.z;
        }
    
        if (p.x < *oldmin) {
            *oldmin = p.x;
        }
    
        if (p.y < *oldmin) {
            *oldmin = p.y;
        }
    
        if (p.z < *oldmin) {
            *oldmin = p.z;
        }
    }
}

namespace tigl 
{

AUTORUN(CTiglExportVtk)
{
    static CCADExporterBuilder<CTiglExportVtk> vtkExporterBuilder;
    CTiglExporterFactory::Instance().RegisterExporter(&vtkExporterBuilder, VtkOptions());
    return true;
}

// Constructor
CTiglExportVtk::CTiglExportVtk(const ExporterOptions& opt)
    : CTiglCADExporter(opt)
{
}

ExporterOptions CTiglExportVtk::GetDefaultOptions() const
{
    return VtkOptions();
}

ShapeExportOptions CTiglExportVtk::GetDefaultShapeOptions() const
{
    return TriangulatedExportOptions(0.001);
}

// Destructor
CTiglExportVtk::~CTiglExportVtk()
{
}

bool CTiglExportVtk::WriteImpl(const std::string &filename) const
{
    TixiDocumentHandle handle;
    tixiCreateDocument("VTKFile", &handle);
    writeVTKHeader(handle);

    size_t nTotalVertices = 0;
    size_t nTotalPolys = 0;

    bool multiplePieces = GlobalExportOptions().Get<bool>("MultiplePieces");
    bool normalsEnabled = GlobalExportOptions().Get<bool>("WriteNormals");

    ComponentTraingMode myMode = NO_INFO;
    if (GlobalExportOptions().Get<bool>("WriteMetaData")) {
        myMode = SEGMENT_INFO;
    }

    if (multiplePieces) {
        for (unsigned int i = 0; i < NShapes(); ++i) {
            // Do the meshing
            PNamedShape pshape = GetShape(i);
            double deflection = GetOptions(i).Get<double>("Deflection");

            const CTiglUIDManager* mgr = GetConfiguration(i) ? &(GetConfiguration(i)->GetUIDManager()) : NULL;
            CTiglTriangularizer mesher(mgr, pshape, deflection, myMode, normalsEnabled);
            const CTiglPolyData& polys = mesher.getTriangulation();
            writeVTKPiece(polys.currentObject(), handle, i + 1);

            nTotalVertices += polys.getTotalVertexCount();
            nTotalPolys += polys.getTotalPolygonCount();
        }
    }
    else {
        double minDeflection = FLT_MAX;
        ListPNamedShape shapes;
        for (unsigned int i = 0; i < NShapes(); ++i) {
            shapes.push_back(GetShape(i));
            double deflection = GetOptions(i).Get<double>("Deflection");
            if (deflection < minDeflection) {
                minDeflection = deflection;
            }
        }
  
        // search for the first uid manager available
        const CTiglUIDManager* mgr = NULL;
        for (unsigned int i = 0; i < NShapes(); ++i) {
            if (GetConfiguration(i)) {
                mgr = &(GetConfiguration(i)->GetUIDManager());
                break;
            }
        }

        PNamedShape groupedShape = CGroupShapes(shapes);
        CTiglTriangularizer mesher(mgr, groupedShape, minDeflection, myMode, normalsEnabled);
        const CTiglPolyData& polys = mesher.getTriangulation();
        writeVTKPiece(polys.currentObject(), handle, 1);

        nTotalVertices += polys.getTotalVertexCount();
        nTotalPolys += polys.getTotalPolygonCount();
    }
    
    if (tixiSaveDocument(handle, filename.c_str())!= SUCCESS) {
        return false;
    }
    LOG(INFO) << "VTK Export succeeded with " << nTotalPolys
              << " polygons and " << nTotalVertices << " vertices." << std::endl;
    return true;
}

void CTiglExportVtk::WritePolys(const CTiglPolyData& polys, const char *filename)
{
    TixiDocumentHandle handle;
    tixiCreateDocument("VTKFile", &handle);
    writeVTKHeader(handle);
    for (unsigned int iObj = 1; iObj <= polys.getNObjects(); ++iObj) {
        writeVTKPiece(polys.getObject(iObj), handle, iObj);
    }
    if (tixiSaveDocument(handle, filename)!= SUCCESS) {
        throw CTiglError("Error saving vtk file!");
    }
    LOG(INFO) << "VTK Export succeeded with " << polys.getTotalPolygonCount()
              << " polygons and " << polys.getTotalVertexCount() << " vertices." << std::endl;
}

void CTiglExportVtk::writeVTKHeader(TixiDocumentHandle& handle)
{
    tixiAddTextAttribute(handle, "/VTKFile", "type", "PolyData");
    tixiAddTextAttribute(handle, "/VTKFile", "version", "0.1");
    tixiAddTextAttribute(handle, "/VTKFile", "byte_order", "LittleEndian");
    tixiAddTextAttribute(handle, "/VTKFile", "compressor", "vtkZLibDataCompressor");

    std::stringstream stream;
    stream << "tigl " << tiglGetVersion();
    tixiCreateElement(handle, "/VTKFile","MetaData");
    tixiAddTextAttribute(handle, "/VTKFile/MetaData", "creator", stream.str().c_str());
    
    tixiCreateElement(handle, "/VTKFile", "PolyData");
}

// writes the polygon data of a surface (in vtk they call it piece)
void CTiglExportVtk::writeVTKPiece(const CTiglPolyObject& co, TixiDocumentHandle& handle, unsigned int iObject)
{
    using std::endl;

    if (co.getNPolygons() == 0) {
        return;
    }

    // count number of vertices - this is not necessarily the number of points
    int nvert = 0;
    for (unsigned int i = 0; i < co.getNPolygons(); ++i) {
        nvert += co.getNPointsOfPolygon(i);
    }

    if (nvert <= 0) {
        return;
    }

    // surface specific stuff
    tixiCreateElement(handle, "/VTKFile/PolyData","Piece");

    const std::string piecepath = "/VTKFile/PolyData/Piece[" + std_to_string(iObject) + "]";

    tixiAddIntegerAttribute(handle, piecepath.c_str(), "NumberOfPoints", co.getNVertices(), "%d");
    tixiAddIntegerAttribute(handle, piecepath.c_str(), "NumberOfVerts",  0, "%d");
    tixiAddIntegerAttribute(handle, piecepath.c_str(), "NumberOfLines",  0, "%d");
    tixiAddIntegerAttribute(handle, piecepath.c_str(), "NumberOfStrips", 0, "%d");
    tixiAddIntegerAttribute(handle, piecepath.c_str(), "NumberOfPolys",  co.getNPolygons(), "%d");

    tixiCreateElement(handle, piecepath.c_str(), "Points");

    //points
    {
        unsigned int nPoints = co.getNVertices();
        std::stringstream stream1;
        stream1 << std::endl <<   "         ";
        double min_coord = DBL_MAX, max_coord = DBL_MIN;
        for (unsigned int i = 0; i < nPoints; ++i) {
            const CTiglPoint& p = co.getVertexPoint(i);
            setMinMax(p, &min_coord, &max_coord);
            stream1 << "    " << std::setprecision(10) << p.x << " " << p.y << " " << p.z << std::endl;
            stream1 << "         ";
        }
        std::string tmpPath = piecepath + "/Points";
        tixiAddTextElement(handle, tmpPath.c_str(), "DataArray", stream1.str().c_str());
        tmpPath += "/DataArray";
        tixiAddTextAttribute(handle, tmpPath.c_str(), "type", "Float64");
        tixiAddTextAttribute(handle, tmpPath.c_str(), "Name", "Points");
        tixiAddTextAttribute(handle, tmpPath.c_str(), "NumberOfComponents", "3");
        tixiAddTextAttribute(handle, tmpPath.c_str(), "format", "ascii");
        tixiAddDoubleAttribute(handle, tmpPath.c_str(), "RangeMin", min_coord ,"%f");
        tixiAddDoubleAttribute(handle, tmpPath.c_str(), "RangeMax", max_coord ,"%f");
    }

    //normals
    if (co.hasNormals()) {
        tixiCreateElement(handle, piecepath.c_str(), "PointData");
        std::string tmpPath = piecepath + "/PointData";
        tixiAddTextAttribute(handle, tmpPath.c_str(), "Normals", "surf_normals");

        std::stringstream stream;
        double min_coord = DBL_MAX, max_coord = DBL_MIN;
        stream << endl  << "        ";
        for (unsigned int i=0; i < co.getNVertices(); ++i) {
             const CTiglPoint& n = co.getVertexNormal(i);
             setMinMax(n, &min_coord, &max_coord);
             stream << "    " << n.x << " " << n.y << " "  << n.z << endl;
             stream <<  "        ";
        }

        tixiAddTextElement(handle, tmpPath.c_str(), "DataArray", stream.str().c_str());
        tmpPath += "/DataArray";
        tixiAddTextAttribute(handle, tmpPath.c_str(), "type", "Float64");
        tixiAddTextAttribute(handle, tmpPath.c_str(), "Name", "surf_normals");
        tixiAddTextAttribute(handle, tmpPath.c_str(), "NumberOfComponents", "3");
        tixiAddTextAttribute(handle, tmpPath.c_str(), "format", "ascii");
        tixiAddDoubleAttribute(handle, tmpPath.c_str(), "RangeMin", min_coord ,"%f");
        tixiAddDoubleAttribute(handle, tmpPath.c_str(), "RangeMax", max_coord ,"%f");
    }

    //polygons
    {
        tixiCreateElement(handle, piecepath.c_str(), "Polys");
        std::stringstream stream2;
        stream2 << std::endl <<   "        ";
        for (unsigned int iPoly = 0; iPoly < co.getNPolygons(); ++iPoly) {
            stream2 <<     "    ";
            for (unsigned int jPoint = 0; jPoint < co.getNPointsOfPolygon(iPoly); ++jPoint ) {
                stream2 << co.getVertexIndexOfPolygon(jPoint, iPoly) << " ";
            }
            stream2  << std::endl <<  "        ";;
        }

        std::string tmpPath = piecepath + "/Polys";
        tixiAddTextElement(handle, tmpPath.c_str(), "DataArray", stream2.str().c_str());
        tmpPath += "/DataArray";
        tixiAddTextAttribute(handle,tmpPath.c_str(), "type", "Int32");
        tixiAddTextAttribute(handle, tmpPath.c_str(), "Name", "connectivity");
        tixiAddTextAttribute(handle, tmpPath.c_str(), "format", "ascii");
        tixiAddIntegerAttribute(handle, tmpPath.c_str(), "RangeMin", 0 ,"%d");
        tixiAddIntegerAttribute(handle, tmpPath.c_str(), "RangeMax", co.getNVertices()-1,"%d");
    }

    //offset
    {
        unsigned int next = 0;
        std::stringstream stream3;
        for (unsigned int i = 0; i < co.getNPolygons(); i ++) {
            if ((i % 10 == 0) && (i != (co.getNPolygons() - 1))) {
                stream3 << endl << "            ";
            }
            next += co.getNPointsOfPolygon(i);
            stream3 << " " << next;
        }
        stream3 << endl << "        ";
        std::string tmpPath = piecepath + "/Polys";
        tixiAddTextElement(handle, tmpPath.c_str(), "DataArray", stream3.str().c_str());
        tmpPath += "/DataArray[2]";
        tixiAddTextAttribute(handle, tmpPath.c_str(), "type", "Int32");
        tixiAddTextAttribute(handle, tmpPath.c_str(), "Name", "offsets");
        tixiAddIntegerAttribute(handle, tmpPath.c_str(), "RangeMin", co.getNPointsOfPolygon(0) ,"%d");
        tixiAddIntegerAttribute(handle, tmpPath.c_str(), "RangeMax", nvert,"%d");
    }
    
    // write cell data
    if (co.getNumberOfPolyRealData() > 0) {
        tixiCreateElement(handle, piecepath.c_str(), "CellData");
        const std::string tmpPath = piecepath + "/CellData";
        
        for (unsigned int iData = 0; iData < co.getNumberOfPolyRealData(); ++iData) {
            const char * dataField = co.getPolyDataFieldName(iData);
            std::stringstream stream;
            for (unsigned long jPoly = 0; jPoly < co.getNPolygons(); ++jPoly) {
                stream << co.getPolyDataReal(jPoly, dataField) << " ";
            }
            tixiAddTextElement(handle, tmpPath.c_str(), "DataArray", stream.str().c_str());
            const std::string path = tmpPath + "/DataArray[" + std_to_string(iData + 1) + "]";
            tixiAddTextAttribute(handle, path.c_str(), "type", "Float64");
            tixiAddTextAttribute(handle, path.c_str(), "Name", dataField);
            tixiAddTextAttribute(handle, path.c_str(), "NumberOfComponents", "1");
            tixiAddTextAttribute(handle, path.c_str(), "format", "ascii");
            tixiAddDoubleAttribute(handle, path.c_str(), "RangeMin", 0. ,"%f");
            tixiAddDoubleAttribute(handle, path.c_str(), "RangeMax", 1. ,"%f");
        }
    }

    // write metadata
    if (co.hasMetadata()) {
        std::stringstream stream4;
        stream4 << endl  << "        ";
        for (unsigned int i = 0; i < co.getNPolygons(); i ++) {
            stream4 << "    " << co.getPolyMetadata(i) << endl;
            stream4 << "        ";
        }
        std::string tmpPath = piecepath + "/Polys";
        tixiAddTextElement(handle, tmpPath.c_str(), "MetaData", stream4.str().c_str());
        tmpPath += "/MetaData";
        tixiAddTextAttribute(handle, tmpPath.c_str(), "elements", co.getMetadataElements() );
    }
}

} // end namespace tigl
