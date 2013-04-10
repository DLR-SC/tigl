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
-count Point und Vertices einfühgen - teils fertig
-Eingefügte werte auf plausibilität prüfen
-translate rotate scale überprüfen
-Material referenz fehlt (pong)

-UP_Axis = Z einfühgen <up_axis>Z_UP</up_axis> in Asset als letztes element
*/

#include "CTiglExportCollada.h"

#include "CTiglPolyData.h"

#include <tixi.h>

#include <cstdio>
#include <ctime>
#include <string>

namespace tigl{

CTiglExportCollada::CTiglExportCollada()
{
}



TiglReturnCode CTiglExportCollada::writeToDisc(CTiglPolyData& polyData, const char * filename)  {

    char col_id[255];
    sprintf(col_id, "dummfile");

    std::stringstream stream1;
    std::stringstream stream2;
    std::stringstream stream3;
    unsigned long count_pos =0, count_norm =0, count_vert =0; // count Points, Normals, Verticies for COLLADA-schema arrays

    ///*
        for(int i = 1; i <= polyData.getNObjects(); ++i){
            CTiglPolyObject& obj = polyData.switchObject(i);

            unsigned long nvert = obj.getNVertices();
            count_vert+=nvert;
            for(int jvert = 0; jvert < nvert; ++jvert){
                // write to vertex list and to normal list
                const CTiglPoint& v = obj.getVertexPoint(jvert);
                const CTiglPoint& n = obj.getVertexNormal(jvert);

                // write v.x, v.y ... with tixi , the same for normals nl

                // VertexPoint
                stream1 << "    " <<  v.x << " " << v.y << " " << v.z << std::endl;
                stream1 << "         ";

                // VertexNormals
                stream2 << "    " <<  n.x << " " << n.y << " " << n.z << std::endl;
                stream2 << "         ";
                count_norm++;
                count_pos++;
            }

            unsigned long ntria = obj.getNPolygons();

            for(int jtria = 0; jtria < ntria; ++jtria){
                // write to triange list
                unsigned int npoints = obj.getNPointsOfPolygon(jtria);
                if( npoints != 3){
                    // display error message
                    cout << "error npoints!=3";
                }

                for(int kpoint = 0; kpoint < npoints; ++kpoint){
                    // get vertex index of polygon
                    unsigned long vindex = obj.getVertexIndexOfPolygon(kpoint, jtria);

                    // write vertex index into list of vertices and normals
                    stream3 << "    " <<  vindex << " " << vindex << " ";
                    count_vert++;
                }
            }


        } //ende for



    // header für COLLADA

    TixiDocumentHandle handle = -1;  //brauch ich wofür???
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
    tixiAddTextElement(handle, "/COLLADA/asset","modified", buffer); // created und modified immer das selbe Datum???



    // Body für COLLADA

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
    // Pos einfühgen - Werte in schleife
    tixiAddTextElement(handle, "/COLLADA/library_geometries/geometry/mesh/source", "float_array", stream1.str().c_str());

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
    tixiAddTextAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/source[2]", "id", tmpstr);    //source[2] für das zweite Element mit dem Bezeichner source

    /* ------ */
    // Normals einfügen - Werte in schleife
    tixiAddTextElement(handle, "/COLLADA/library_geometries/geometry/mesh/source[2]", "float_array", stream2.str().c_str());

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
    tixiAddIntegerAttribute(handle, "/COLLADA/library_geometries/geometry/mesh/triangles", "count", count_vert, "%d"); //AddInteger count dreiecke = ntria
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
    // Vertices einfühgen, Werte in Schleifendurchlauf
    tixiAddTextElement(handle, "/COLLADA/library_geometries/geometry/mesh//triangles", "p", stream3.str().c_str());

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


// Backup von Matins Schleife
/*
    for(unsigned int i = 1; i < polyData.getNObjects(); ++i){
        CTiglPolyObject& obj = polyData.switchObject(i);
test1++;
        unsigned long nvert = obj.getNVertices();
        for(int jvert = 0; jvert < nvert; ++jvert){
            // write to vertex list and to normal list
            CTiglPoint& v = obj.getVertexPoint(jvert);
            CTiglPoint& n = obj.getVertexNormal(jvert);

            // write v.x, v.y ... with tixi , the same for normals nl
            test++;
            cout << "X " << v.x << "Y " << v.y << "X " << v.z << endl;
            cout << "n.X " << n.x << "nY " << n.y << "nZ" << n.z << endl;

        }


        unsigned long ntria = obj.getNPolygons();
        for(int jtria = 0; jtria < ntria; ++jtria){
            // write to triange list
            unsigned int npoints = obj.getNPointsOfPolygon(jtria);
            if( npoints != 3){
                // display error message
                cout << "error npoints!=3";
            }

            for(int kpoint = 0; kpoint < npoints; ++kpoint){
                // get vertex index of polygon
                unsigned long vindex = obj.getVertexIndexOfPolygon(kpoint, jtria);

                // write vertex index into list of vertices and normals
                cout << vindex << endl;
            }
        }


    } //ende for
*/
