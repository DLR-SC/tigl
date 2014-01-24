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

/**
 * @brief The CTiglShapeCache class acts as a storage for all kinds of intermediate 
 * shape results in order to improve speed of reoccurent calculations.
 */

#ifndef CTIGLSHAPECACHE_H
#define CTIGLSHAPECACHE_H

#include <map>
#include <string>
#include <TopoDS_Shape.hxx>

namespace tigl 
{

class CTiglShapeCache
{
public:
    typedef std::map<std::string, TopoDS_Shape> ShapeContainer;
    
    CTiglShapeCache();
    
    /// Inserts a shape with the given id into the cache
    void Insert(const TopoDS_Shape& shape, const std::string& id);
    
    /// Returns the shape with the given id and an optional number (if multiple shapes 
    /// with the same id were added). If the shape is not found, a nullshape/empty shape is returned
    TopoDS_Shape& GetShape(const std::string& id, unsigned int inumber = 0);
    
    /// Returns number of shapes with the same id
    unsigned int GetNShapesOfType(const std::string& id) const;
    
    /// Returns total number of shapes in map
    unsigned int GetNShape() const;
    
    /// Removes alles shapes with the given id from the cache
    void Remove(const std::string& id);
    
    /// Empties the cache
    void Clear();
    
    /// Returns the underlying map for direct access
    ShapeContainer& GetContainer();
    
private:
    void Reset();
    
    ShapeContainer shapeContainer;
    TopoDS_Shape nullShape;
};

} // namespace tigl

#endif // CTIGLSHAPECACHE_H
