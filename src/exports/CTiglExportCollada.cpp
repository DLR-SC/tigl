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
-Bezeichnung (Box/Filname) korigieren
-translate rotate scale überprüfen
-Material referenz fehlt (pong)

- extract col_id from filename (no absolute file path)
- change, that multiple objects are stored as different collada objects

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

#ifdef _MSC_VER
    #define snprintf _snprintf
#endif

namespace tigl
{

CTiglExportCollada::CTiglExportCollada(CCPACSConfiguration& config) : myconfig(config)
{
}


TiglReturnCode CTiglExportCollada::exportFuselage(const std::string& fuselageUID, const std::string &filename, const double deflection)
{
    CTiglAbstractPhysicalComponent & component = myconfig.GetFuselage(fuselageUID);
    TopoDS_Shape loft = component.GetLoft()->Shape();
    
    return exportShape(loft, fuselageUID, filename, deflection);
}

TiglReturnCode CTiglExportCollada::exportWing(const std::string& wingUID, const std::string &filename, const double deflection)
{
    CTiglAbstractPhysicalComponent & component = myconfig.GetWing(wingUID);
    TopoDS_Shape loft = component.GetLoft()->Shape();
    
    return exportShape(loft, wingUID, filename, deflection);
}

TiglReturnCode CTiglExportCollada::exportShape(TopoDS_Shape& shape, const std::string& shapeID,  const std::string& filename, const double deflection)
{
    // mesh 
    CTiglTriangularizer t(shape, deflection, false);
    return writeToDisc(t, shapeID.c_str(), filename.c_str());
}

TiglReturnCode CTiglExportCollada::writeToDisc(CTiglPolyData& polyData, const char* col_id, const char * filename)  
{

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

    // COLLADA header

    TixiDocumentHandle handle = -1;  
    tixiCreateDocument("COLLADA", &handle);
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

    // Body
    tixiCreateElement(handle,"/COLLADA","library_geometries");

    tixiCreateElement(handle,"/COLLADA/library_geometries", "geometry");
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry", "id", col_id);
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry", "name", col_id);

      tixiCreateElement(handle, "/COLLADA/library_geometries/geometry","mesh");

    tixiCreateElement(handle, "/COLLADA/library_geometries/geometry/mesh", "source");
    char tmpstr[1024];
    snprintf(tmpstr, 1024, "%s-Pos", col_id);
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/source", "id", tmpstr);

    /* ------ */
    // Vertices
    tixiAddTextElement(handle, "/COLLADA/library_geometries/geometry/mesh/source", "float_array", stream_verts.str().c_str());

    tixiAddIntegerAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/source/float_array", "count", count_pos*3, "%d"); // count(Pos)
    snprintf(tmpstr, 1024, "%s-Pos-array",col_id);
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/source/float_array", "id", tmpstr);

    tixiCreateElement(handle,"/COLLADA/library_geometries/geometry/mesh/source", "technique_common");

    tixiCreateElement(handle,"/COLLADA/library_geometries/geometry/mesh/source/technique_common","accessor");
    tixiAddIntegerAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor","count",count_pos,"%d"); // count Pos/stride
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor","offset","0");
    snprintf(tmpstr, 1024, "#%s-Pos-array",col_id);
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor","source",tmpstr);
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor","stride","3");

    tixiCreateElement(handle, "/COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor", "param");
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor/param", "name", "X");
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor/param", "type", "float");

    tixiCreateElement(handle, "/COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor", "param");
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor/param[2]", "name", "Y");
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor/param[2]", "type", "float");

    tixiCreateElement(handle, "/COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor", "param");
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor/param[3]", "name", "Z");
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source/technique_common/accessor/param[3]", "type", "float");

    tixiCreateElement(handle, "/COLLADA/library_geometries/geometry/mesh", "source");
    snprintf(tmpstr, 1024, "%s-normals",col_id);
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/source[2]", "id", tmpstr);

    /* ------ */
    // Normals 
    tixiAddTextElement(handle, "/COLLADA/library_geometries/geometry/mesh/source[2]", "float_array", stream_normals.str().c_str());

    tixiAddIntegerAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/source[2]/float_array", "count", (count_norm*3), "%d");
    snprintf(tmpstr, 1024, "%s-normals-array",col_id);
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/source[2]/float_array", "id", tmpstr);

    tixiCreateElement(handle,"/COLLADA/library_geometries/geometry/mesh/source[2]", "technique_common");

    tixiCreateElement(handle,"/COLLADA/library_geometries/geometry/mesh/source[2]/technique_common","accessor");
    tixiAddIntegerAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source[2]/technique_common/accessor","count",count_pos,"%d");
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source[2]/technique_common/accessor","offset","0");
    snprintf(tmpstr, 1024, "#%s-normals-array",col_id);
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source[2]/technique_common/accessor","source",tmpstr);
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source[2]/technique_common/accessor","stride","3");

    tixiCreateElement(handle, "/COLLADA/library_geometries/geometry/mesh/source[2]/technique_common/accessor", "param");
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source[2]/technique_common/accessor/param", "name", "X");
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source[2]/technique_common/accessor/param", "type", "float");

    tixiCreateElement(handle, "/COLLADA/library_geometries/geometry/mesh/source[2]/technique_common/accessor", "param");
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source[2]/technique_common/accessor/param[2]", "name", "Y");
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source[2]/technique_common/accessor/param[2]", "type", "float");

    tixiCreateElement(handle, "/COLLADA/library_geometries/geometry/mesh/source[2]/technique_common/accessor", "param");
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source[2]/technique_common/accessor/param[3]", "name", "Z");
    tixiAddTextAttribute(handle,"/COLLADA/library_geometries/geometry/mesh/source[2]/technique_common/accessor/param[3]", "type", "float");

    tixiCreateElement(handle, "/COLLADA/library_geometries/geometry/mesh", "vertices");
    snprintf(tmpstr, 1024, "%s-vertices",col_id);
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/vertices", "id", tmpstr);

    tixiCreateElement(handle, "/COLLADA/library_geometries/geometry/mesh/vertices","input");
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/vertices/input","semantic","POSITION");
    snprintf(tmpstr, 1024, "#%s-Pos",col_id);
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/vertices/input","source",tmpstr);

    tixiCreateElement(handle, "/COLLADA/library_geometries/geometry/mesh", "triangles");
    tixiAddIntegerAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/triangles", "count", count_vert, "%d");
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/triangles", "material", "WHITE");

    tixiCreateElement(handle, "/COLLADA/library_geometries/geometry/mesh/triangles","input");
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/triangles/input","offset","0");
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/triangles/input","semantic","VERTEX");
    snprintf(tmpstr, 1024, "#%s-vertices",col_id);
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/triangles/input","source",tmpstr);

    tixiCreateElement(handle, "/COLLADA/library_geometries/geometry/mesh/triangles","input");
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/triangles/input[2]","offset","1");
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/triangles/input[2]","semantic","NORMAL");
    snprintf(tmpstr, 1024, "#%s-normals",col_id);
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/triangles/input[2]","source",tmpstr);

    /* ------ */
    // Insert vertex data
    tixiAddTextElement(handle, "/COLLADA/library_geometries/geometry/mesh//triangles", "p", stream_trians.str().c_str());

    tixiCreateElement(handle, "/COLLADA", "library_visual_scenes");

    tixiCreateElement(handle, "/COLLADA/library_visual_scenes", "visual_scene");

    tixiAddTextAttribute(handle, "/COLLADA/library_visual_scenes/visual_scene", "id", "DefaultScene");

    tixiCreateElement(handle, "/COLLADA/library_visual_scenes/visual_scene", "node");
    tixiAddTextAttribute(handle, "/COLLADA/library_visual_scenes/visual_scene/node", "id", col_id);
    tixiAddTextAttribute(handle, "/COLLADA/library_visual_scenes/visual_scene/node", "name", col_id);

    tixiAddTextElement(handle, "/COLLADA/library_visual_scenes/visual_scene/node", "translate", "0 0 0");
    tixiAddTextElement(handle, "/COLLADA/library_visual_scenes/visual_scene/node", "rotate", "0 0 1 0");
    tixiAddTextElement(handle, "/COLLADA/library_visual_scenes/visual_scene/node", "rotate", "0 1 0 0");
    tixiAddTextElement(handle, "/COLLADA/library_visual_scenes/visual_scene/node", "rotate", "1 0 0 0");
    tixiAddTextElement(handle, "/COLLADA/library_visual_scenes/visual_scene/node", "scale", "1 1 1");

    tixiCreateElement(handle, "/COLLADA/library_visual_scenes/visual_scene/node", "instance_geometry");
    snprintf(tmpstr, 1024, "#%s",col_id);
    tixiAddTextAttribute(handle, "/COLLADA/library_visual_scenes/visual_scene/node/instance_geometry", "url", tmpstr);

    tixiCreateElement(handle, "/COLLADA/library_visual_scenes/visual_scene/node/instance_geometry", "bind_material");

    tixiCreateElement(handle, "/COLLADA/library_visual_scenes/visual_scene/node/instance_geometry/bind_material", "technique_common");

    tixiCreateElement(handle, "/COLLADA/library_visual_scenes/visual_scene/node/instance_geometry/bind_material/technique_common", "instance_material");
    tixiAddTextAttribute(handle, "/COLLADA/library_visual_scenes/visual_scene/node/instance_geometry/bind_material/technique_common/instance_material","symbol", "WHITE");
    tixiAddTextAttribute(handle, "/COLLADA/library_visual_scenes/visual_scene/node/instance_geometry/bind_material/technique_common/instance_material","target", "#whiteMaterial");
    // #whiteMaterial = NULL, da nicht implementiert. Fehlt noch oben im Header-Bereich ... Pong, def collor usw.

    tixiSaveDocument(handle, filename);

    return TIGL_SUCCESS;
}

} // namespace tigl
