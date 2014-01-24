/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-06-18 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglShapeCache.h"
#include <sstream>

namespace 
{
    std::string mangleID(const std::string & id, unsigned int number)
    {
        std::stringstream stream;
        stream << id << "::" << number;
        return stream.str();
    }
}

namespace tigl 
{

CTiglShapeCache::CTiglShapeCache() 
{
    Reset();
}

void CTiglShapeCache::Insert(const TopoDS_Shape &shape, const std::string &id)
{
    int nshapes = GetNShapesOfType(id);
    shapeContainer[mangleID(id,nshapes)] = shape;
}

TopoDS_Shape& CTiglShapeCache::GetShape(const std::string &id, unsigned int inumber) 
{
    ShapeContainer::iterator it = shapeContainer.find(mangleID(id, inumber));
    if (it == shapeContainer.end()) {
        return nullShape;
    }
    else {
        return it->second;
    }
}

unsigned int CTiglShapeCache::GetNShapesOfType(const std::string &id) const 
{
    int nshapes = 0;
    while(true){
        ShapeContainer::const_iterator it = shapeContainer.find(mangleID(id, nshapes));
        if (it != shapeContainer.end()) {
            nshapes++;
        }
        else {
            break;
        }
    }
    
    return nshapes;
}

unsigned int CTiglShapeCache::GetNShape() const 
{
    return shapeContainer.size();
}


void CTiglShapeCache::Clear()
{
    shapeContainer.clear();
}

void CTiglShapeCache::Remove(const std::string& id) 
{
    int nshapes = 0;
    while(true){
        ShapeContainer::iterator it = shapeContainer.find(mangleID(id, nshapes));
        if (it != shapeContainer.end()) {
            nshapes++;
            shapeContainer.erase(it);
        }
        else {
            break;
        }
    }
}

void CTiglShapeCache::Reset() 
{
    nullShape.Nullify();
    Clear();
}

CTiglShapeCache::ShapeContainer& CTiglShapeCache::GetContainer() 
{
    return shapeContainer;
}

} // namespace tigl
