/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-02-18 Martin Siggel <Martin.Siggel@dlr.de>
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
* @file CTiglTriangularizer.h
* @brief Helper class to traingularize tigl objects and TopoDS_Shapes
*/

#ifndef CTIGLTRIANGULARIZER_H
#define CTIGLTRIANGULARIZER_H

#include "CTiglPolyData.h"

class TopoDS_Shape;

namespace tigl {

class CTiglTriangularizer : public CTiglPolyData
{
public:
    CTiglTriangularizer();
    CTiglTriangularizer(TopoDS_Shape&, bool useMultipleObjects = false);
    CTiglTriangularizer(class CTiglAbstractPhysicalComponent&);
    
    void useMultipleObjects(bool);
    
private:
    int triangularizeShape(const TopoDS_Shape &);
    int computeVTKMetaData(class CCPACSWing&);
    
    bool _useMultipleObjects;
};

}

#endif // CTIGLTRIANGULARIZER_H
