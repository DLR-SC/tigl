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

namespace tigl 
{

CTiglShapeCache::CTiglShapeCache() 
{
    Reset();
}

void CTiglShapeCache::Insert(const TopoDS_Shape &shape, const std::string& id)
{
    shapeContainer[id] = shape;
}

TopoDS_Shape& CTiglShapeCache::GetShape(const std::string& id) 
{
    ShapeContainer::iterator it = shapeContainer.find(id);
    if (it == shapeContainer.end()) {
        return nullShape;
    }
    else {
        return it->second;
    }
}

/// Returns true, if the shape with id is in the cache
bool CTiglShapeCache::HasShape(const std::string& id)
{
    ShapeContainer::iterator it = shapeContainer.find(id);
    return it != shapeContainer.end();
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
    ShapeContainer::iterator it = shapeContainer.find(id);
    if (it != shapeContainer.end()) {
        shapeContainer.erase(it);
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
