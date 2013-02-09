/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-02-09 Martin Siggel <Martin.Siggel@dlr.de>
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
*
* @file CTiglPolyData.h
* @brief Handles polygon data for export and rendering usage
*/

#ifndef VTKPOLYDATA_H
#define VTKPOLYDATA_H

#include <ostream>
#include <vector>
#include "CTiglPoint.h"

class PolyDataImpl;

namespace tigl {

class CTiglPolygon{
public:
    CTiglPolygon();

    void addPoint(const CTiglPoint&);

    // returns number of points inside polygon
    unsigned int getNPoints() const;

    CTiglPoint& getPoint(unsigned int index);
    const CTiglPoint& getPointConst(unsigned int index) const;

    // sets a metadata text, can be used for vtk export
    void setMetadata(const char *);

    // returns metadata text
    const char * getMetadata() const;

private:
    std::string _metadata;
    std::vector<CTiglPoint> _points;
};

class CTiglPolyData{
public:
    CTiglPolyData();
    ~CTiglPolyData();
    
    // adds a point to a polygon with a specific id
    void addPoint(const class CTiglPoint& p, int id);

    // adds a polygon to the object
    void addPolygon(const class CTiglPolygon&);
    
    void writeVTK(const char * filename);
    void printVTK();
    
private:
    PolyDataImpl * impl;
};

}

#endif // VTKPOLYDATA_H
