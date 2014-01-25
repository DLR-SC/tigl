/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-10-12 Martin Siggel <Martin.Siggel@dlr.de>
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


#include "CHotsoseMeshReader.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

#include <fstream>

#include "CTiglLogging.h"

CHotsoseMeshReader::CHotsoseMeshReader()
{
}

TiglReturnCode CHotsoseMeshReader::readFromFile(const char * filename, tigl::CTiglPolyData& mesh)
{
    using namespace boost::algorithm;
    mesh.currentObject().enableNormals(false);

    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG(ERROR) << "Cannot read file " << filename;
        return TIGL_OPEN_FAILED;
    }

    std::string line;
    int linenr = 0;
    int lastnr = 0;
    int polycount = 0;
    tigl::CTiglPolygon* poly = new tigl::CTiglPolygon;
    while (std::getline(file, line)) {

        std::vector<std::string> list;
        trim(line);
        split(list, line, boost::is_any_of("\t "), token_compress_on);
        if (list.size() == 4) {
           // parse polygon number
            int pnr = boost::lexical_cast<int>(list.at(0));
            if (pnr != lastnr && poly->getNPoints() > 0) {
                mesh.currentObject().addPolygon(*poly);
                // make a mirrored copy of the poly
                tigl::CTiglPolygon polymirr;
                for (int i = poly->getNPoints()-1; i >= 0; --i) {
                    tigl::CTiglPoint pnt = poly->getPoint(i);
                    pnt.y *= -1.;
                    polymirr.addPoint(pnt);
                }
                mesh.currentObject().addPolygon(polymirr);

                delete poly;
                poly = new tigl::CTiglPolygon;
                polycount++;
            }

            lastnr = pnr;

            tigl::CTiglPoint p;
            std::string sx = list.at(1);
            std::string sy = list.at(2);
            std::string sz = list.at(3);

            // gghs number output might be something like 3.4100D-01
            // convert it into standard form (with E)
            replace_all(sx, "D", "E");
            replace_all(sy, "D", "E");
            replace_all(sz, "D", "E");
            p.x = boost::lexical_cast<double>(sx);
            p.y = boost::lexical_cast<double>(sy);
            p.z = boost::lexical_cast<double>(sz);

            poly->addPoint(p);
        }

        linenr++;
    }
    if (poly && poly->getNPoints() > 0) {
        mesh.currentObject().addPolygon(*poly);
        tigl::CTiglPolygon polymirr;
        for (int i = poly->getNPoints()-1; i >= 0; --i) {
            tigl::CTiglPoint pnt = poly->getPoint(i);
            pnt.y *= -1.;
            polymirr.addPoint(pnt);
        }
        mesh.currentObject().addPolygon(polymirr);
    }
    delete poly;

    return TIGL_SUCCESS;
}
