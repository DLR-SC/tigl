/* 
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-04-08 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef TIGLINTERACTIVESHAPEMANAGER_H
#define TIGLINTERACTIVESHAPEMANAGER_H

#include "PNamedShape.h"
#include <AIS_Shape.hxx>
#include <map>
#include <string>
#include <vector>

typedef std::vector<Handle(AIS_Shape)> IObjectList;

struct ShapeEntry
{
public:
    PNamedShape shape;
    std::vector<Handle(AIS_Shape)> aisObjects;
};

class InteractiveShapeManager
{
public:
    InteractiveShapeManager();

    PNamedShape GetShapeFromName(const std::string& name);
    PNamedShape GetShapeFromIObject(Handle(AIS_Shape) obj);
    IObjectList GetIObjectsFromShapeName(const std::string& name) const;

    std::vector<PNamedShape> GetAllShapes() const;

    // removes the shape and all interactive objects belonging to the shape
    void removeObject(const std::string& name);
    void clear();

    // removes only the interactive object from the shape manager
    void removeObject(Handle(AIS_Shape) obj);
    void addObject(PNamedShape shape, const Handle(AIS_Shape) iObject);
    void addObject(std::string uid, Handle(AIS_Shape) iObject);
    
    bool HasShapeEntry(const std::string& name) const;
    bool GetVisibility(const std::string& name) const;

    std::vector<std::string> GetDisplayedShapeNames() const;

private:
    ShapeEntry& GetShapeEntry(const std::string& name);

    std::map<std::string, ShapeEntry> _shapeEntries;
    typedef std::map<std::string, ShapeEntry> ShapeMap;
    typedef ShapeMap::iterator ShapeIterator;
    std::map<Handle(AIS_Shape), std::string> _names;
    typedef std::map<Handle(AIS_Shape), std::string>::iterator NameIterator;
};

#endif // TIGLINTERACTIVESHAPEMANAGER_H
