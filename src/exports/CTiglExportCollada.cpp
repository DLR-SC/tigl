/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-03-19 Volker Poddey <Volker.Poddey@dlr.de>
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


/*
TODO:
-translate rotate scale überprüfen
-Material referenz fehlt (pong)
*/

#include "CTiglExportCollada.h"

#include "CTiglPolyData.h"
#include "CTiglTriangularizer.h"
#include "CCPACSConfiguration.h"

#include <tixi.h>

#include <cstdio>
#include <ctime>
#include <string>
#include <cassert>

// OpenCASCADE
#include <TopoDS_Shape.hxx>
#include <BRepTools.hxx>

namespace 
{
    template <typename T>
    std::string toStr ( T Number )
    {
        std::ostringstream ss;
        ss << Number;
        return ss.str();
    }
}

namespace tigl
{

CTiglExportCollada::CTiglExportCollada()
{
}

void CTiglExportCollada::AddShape(PNamedShape shape, double deflection)
{
    if (!shape) {
        return;
    }
    
    _shapes.push_back(shape);
    _deflects.push_back(deflection);
}


bool writeHeader(TixiDocumentHandle handle)
{
    tixiAddTextAttribute(handle, "/COLLADA", "xmlns", "http://www.collada.org/2008/03/COLLADASchema");
    tixiAddTextAttribute(handle, "/COLLADA", "version", "1.5.0");

    tixiCreateElement(handle, "/COLLADA", "asset");

    tixiCreateElement(handle, "/COLLADA/asset", "contributor");

    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (buffer,80,"%Y-%m-%dT%H:%S:%MZ",timeinfo);
    tixiAddTextElement(handle, "/COLLADA/asset","created",  buffer);
    tixiAddTextElement(handle, "/COLLADA/asset","modified", buffer); 
    tixiAddTextElement(handle, "/COLLADA/asset","up_axis", "Z_UP");
    
    tixiCreateElement(handle, "/COLLADA/asset", "unit");
    tixiAddTextAttribute(handle, "/COLLADA/asset/unit", "name", "meters");
    tixiAddDoubleAttribute(handle, "/COLLADA/asset/unit", "meter", 1.0, "%f");
    
    return true;
}

bool writeMeshArray(TixiDocumentHandle handle, std::string meshPath, int sourceIndex, std::string id, std::string data, int dataCount)
{
    tixiCreateElement(handle, meshPath.c_str(), "source");
    
    std::string meshSourcePath = meshPath + "/source[" + toStr(sourceIndex) + "]";
    tixiAddTextAttribute(handle, meshSourcePath.c_str(), "id", id.c_str());

    // Write the data
    tixiAddTextElement(handle, meshSourcePath.c_str(), "float_array", data.c_str());
    tixiAddIntegerAttribute(handle, (meshSourcePath + "/float_array").c_str(), "count", dataCount*3, "%d"); // count(Pos)
    
    std::string arrayid = id + "-array";
    tixiAddTextAttribute(handle, (meshSourcePath + "/float_array").c_str(), "id", arrayid.c_str());

    tixiCreateElement(handle, meshSourcePath.c_str(), "technique_common");

    std::string techniqueCommonPath = meshSourcePath + "/technique_common";
    tixiCreateElement(handle, techniqueCommonPath.c_str(),"accessor");
    
    std::string accessorPath = techniqueCommonPath + "/accessor";
    tixiAddIntegerAttribute(handle, accessorPath.c_str(),"count", dataCount,"%d"); // count Pos/stride
    tixiAddTextAttribute(handle,accessorPath.c_str(),"offset","0");
    
    arrayid = "#" + arrayid;
    tixiAddTextAttribute(handle, accessorPath.c_str(),"source", arrayid.c_str());
    tixiAddTextAttribute(handle, accessorPath.c_str(),"stride","3");

    tixiCreateElement(handle, accessorPath.c_str(), "param");
    std::string paramPath = accessorPath + "/param[1]";
    tixiAddTextAttribute(handle, paramPath.c_str(), "name", "X");
    tixiAddTextAttribute(handle, paramPath.c_str(), "type", "float");

    tixiCreateElement(handle, accessorPath.c_str(), "param");
    paramPath = accessorPath + "/param[2]";
    tixiAddTextAttribute(handle, paramPath.c_str(), "name", "Y");
    tixiAddTextAttribute(handle, paramPath.c_str(), "type", "float");

    tixiCreateElement(handle, accessorPath.c_str(), "param");
    paramPath = accessorPath + "/param[3]";
    tixiAddTextAttribute(handle, paramPath.c_str(), "name", "Z");
    tixiAddTextAttribute(handle, paramPath.c_str(), "type", "float");
    
    return true;
}

bool writeGeometryMesh(TixiDocumentHandle handle, CTiglPolyData& polyData, std::string col_id, int& geometryIndex)
{
    // create vertex, normal and triangle strings
    std::stringstream stream_verts;
    std::stringstream stream_normals;
    std::stringstream stream_trians;
    unsigned long count_pos =0, count_norm =0, count_vert =0; // count Points, Normals, Verticies for COLLADA-schema arrays

    for (unsigned int i = 1; i <= polyData.getNObjects(); ++i) {
        CTiglPolyObject& obj = polyData.switchObject(i);

        unsigned long nvert = obj.getNVertices();
        count_vert+=nvert;
        for (unsigned long jvert = 0; jvert < nvert; ++jvert) {
            const CTiglPoint& v = obj.getVertexPoint(jvert);
            const CTiglPoint& n = obj.getVertexNormal(jvert);

            // VertexPoint
            stream_verts <<  v.x << " " << v.y << " " << v.z << " ";

            // VertexNormals
            stream_normals <<  n.x << " " << n.y << " " << n.z << " ";
            count_norm++;
            count_pos++;
        }

        unsigned long ntria = obj.getNPolygons();

        for (unsigned long jtria = 0; jtria < ntria; ++jtria) {
            unsigned long npoints = obj.getNPointsOfPolygon(jtria);
            // skip all polygons that aren't triangles
            if (npoints < 3) {
                // we currently dont export lines
                continue;
            }
            else if (npoints > 3) {
                LOG(WARNING) << "Polygons with more than 3 vertices are currently not supported by CTiglExportCollada!" << endl;
                continue;
            }

            for (unsigned long kpoint = 0; kpoint < npoints; ++kpoint) {
                // get vertex index of polygon
                unsigned long vindex = obj.getVertexIndexOfPolygon(kpoint, jtria);

                // write vertex index into list of vertices and normals
                stream_trians << vindex << " " << vindex << " ";
            }
            count_vert++;
        }
    } //end for objects
    
    if (tixiCreateElement(handle,"/COLLADA/library_geometries", "geometry") != SUCCESS) {
       return false;
    }
    
    std::string geometryPath  = "/COLLADA/library_geometries/geometry[" +  toStr(geometryIndex) + "]";
    
    tixiAddTextAttribute(handle, geometryPath.c_str(), "id", col_id.c_str());
    tixiAddTextAttribute(handle, geometryPath.c_str(), "name", col_id.c_str());

    tixiCreateElement(handle, geometryPath.c_str(), "mesh");
    std::string meshPath = geometryPath  + "/mesh";

    // write vertices and normals
    writeMeshArray(handle, meshPath, 1, col_id + "-mesh-positions", stream_verts.str(), count_pos);
    writeMeshArray(handle, meshPath, 2, col_id + "-mesh-normals", stream_normals.str(), count_norm);
    
    tixiCreateElement(handle, meshPath.c_str(), "vertices");
    std::string vertsPath = meshPath + "/vertices";
    std::string id = col_id + "-mesh-vertices";
    tixiAddTextAttribute(handle, vertsPath.c_str(), "id", id.c_str());

    tixiCreateElement(handle, vertsPath.c_str(),"input");
    std::string inputPath = vertsPath + "/input";
    tixiAddTextAttribute(handle, inputPath.c_str(),"semantic","POSITION");
    id = "#" + col_id + "-mesh-positions";
    tixiAddTextAttribute(handle, inputPath.c_str(),"source", id.c_str());

    tixiCreateElement(handle, meshPath.c_str(), "triangles");
    std::string trianPath = meshPath + "/triangles";
    tixiAddIntegerAttribute(handle, trianPath.c_str(), "count", count_vert, "%d");
    tixiAddTextAttribute(handle, trianPath.c_str(), "material", "WHITE");

    tixiCreateElement(handle, trianPath.c_str(),"input");
    inputPath = trianPath + "/input";
    tixiAddTextAttribute(handle, inputPath.c_str(),"offset","0");
    tixiAddTextAttribute(handle, inputPath.c_str(),"semantic","VERTEX");
    id = "#" + col_id + "-mesh-vertices";
    tixiAddTextAttribute(handle, inputPath.c_str(),"source", id.c_str());

    tixiCreateElement(handle, trianPath.c_str(),"input");
    inputPath = trianPath + "/input[2]";
    tixiAddTextAttribute(handle, inputPath.c_str(),"offset","1");
    tixiAddTextAttribute(handle, inputPath.c_str(),"semantic","NORMAL");
    id = "#" + col_id + "-mesh-normals";
    tixiAddTextAttribute(handle, inputPath.c_str(),"source", id.c_str());

    /* ------ */
    // Insert vertex data
    tixiAddTextElement(handle, trianPath.c_str(), "p", stream_trians.str().c_str());
    
    geometryIndex++;
    return true;
}

bool writeSceneNode(TixiDocumentHandle handle, std::string scenePath, std::string nodeName, std::string meshID, int& nodeIndex)
{
    tixiCreateElement(handle, scenePath.c_str(), "node");
    
    std::string nodePath = scenePath + "/node[" + toStr(nodeIndex) + "]";
    nodeIndex++;
    tixiAddTextAttribute(handle, nodePath.c_str(), "id",   nodeName.c_str());
    tixiAddTextAttribute(handle, nodePath.c_str(), "name", nodeName.c_str());

    tixiAddTextElement(handle, nodePath.c_str(), "translate", "0 0 0");
    tixiAddTextElement(handle, nodePath.c_str(), "rotate", "0 0 1 0");
    tixiAddTextElement(handle, nodePath.c_str(), "rotate", "0 1 0 0");
    tixiAddTextElement(handle, nodePath.c_str(), "rotate", "1 0 0 0");
    tixiAddTextElement(handle, nodePath.c_str(), "scale", "1 1 1");

    tixiCreateElement(handle, nodePath.c_str(), "instance_geometry");
    std::string instGeomPath = nodePath + "/instance_geometry";
    std::string url = "#" + meshID;
    tixiAddTextAttribute(handle, instGeomPath.c_str(), "url", url.c_str());

    tixiCreateElement(handle, instGeomPath.c_str(), "bind_material");
    std::string bindMatPath = instGeomPath + "/bind_material";

    tixiCreateElement(handle, bindMatPath.c_str(), "technique_common");
    std::string techCommPath = bindMatPath + "/technique_common";

    tixiCreateElement(handle, techCommPath.c_str(), "instance_material");
    std::string instMatPath = techCommPath + "/instance_material";
    
    tixiAddTextAttribute(handle, instMatPath.c_str(),"symbol", "WHITE");
    tixiAddTextAttribute(handle, instMatPath.c_str(),"target", "#whiteMaterial");
    
    return true;
}


TiglReturnCode CTiglExportCollada::Write(const std::string& filename)
{
    TixiDocumentHandle handle = -1;  
    if (tixiCreateDocument("COLLADA", &handle) != SUCCESS) {
        return TIGL_ERROR;
    }
    
    writeHeader(handle);

    // Body
    tixiCreateElement(handle,"/COLLADA","library_geometries");

    // write object mesh info
    int geomIndex = 1;
    for (unsigned int i = 0; i < _shapes.size(); ++i) {
        // Do the meshing
        PNamedShape pshape = _shapes[i];
        double deflection = _deflects[i];
        CTiglTriangularizer polyData(pshape->Shape(), deflection);
        
        writeGeometryMesh(handle, polyData, pshape->Name(), geomIndex);
    }
    
    // write the scene and link object to geometry
    tixiCreateElement(handle, "/COLLADA", "library_visual_scenes");
    tixiCreateElement(handle, "/COLLADA/library_visual_scenes", "visual_scene");
    tixiAddTextAttribute(handle, "/COLLADA/library_visual_scenes/visual_scene", "id", "DefaultScene");

    // add each object to the scene
    int nodeIndex = 1;
    for (unsigned int i = 0; i < _shapes.size(); ++i) {
        PNamedShape pshape = _shapes[i];
        // Todo: insert transformation matrix also
        writeSceneNode(handle, "/COLLADA/library_visual_scenes/visual_scene", pshape->Name(), pshape->Name(), nodeIndex);
    }


    if (tixiSaveDocument(handle, filename.c_str()) != SUCCESS) {
        LOG(ERROR) << "Cannot save collada file " << filename;
        return TIGL_ERROR;
    }

    return TIGL_SUCCESS;
}

} // namespace tigl
