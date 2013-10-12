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
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QStringList>

#include "CTiglLogger.h"

CHotsoseMeshReader::CHotsoseMeshReader()
{
}

TiglReturnCode CHotsoseMeshReader::readFromFile(const char * filename, tigl::CTiglPolyData& mesh){
    mesh.currentObject().enableNormals(false);

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        LOG(ERROR) << "Cannot read file " << filename;
        return TIGL_OPEN_FAILED;
    }

    QTextStream in(&file);
    QString line = in.readLine();

    int linenr = 0;
    int lastnr = 0;
    int polycount = 0;
    tigl::CTiglPolygon* poly = new tigl::CTiglPolygon;
    while (!line.isNull()) {
        line = in.readLine();

        QStringList list = line.split(" ", QString::SkipEmptyParts);
        if(list.size() == 4){
            // parse polygon number
            int pnr = list.at(0).toInt();
            if (pnr != lastnr && poly->getNPoints() > 0){
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
            QString sx = list.at(1);
            QString sy = list.at(2);
            QString sz = list.at(3);

            // gghs number output might be something like 3.4100D-01
            // convert it into standard form (with E)
            p.x = sx.replace("D", "E").toDouble();
            p.y = sy.replace("D", "E").toDouble();
            p.z = sz.replace("D", "E").toDouble();

            poly->addPoint(p);
        }

        linenr++;
    }
    if (poly && poly->getNPoints() > 0){
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
