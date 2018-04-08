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

#include "TIGLInteractiveShapeManager.h"
#include "CNamedShape.h"
#include "Handle_AIS_InteractiveObject.hxx"

#include <algorithm>

//#define DEBUG

InteractiveShapeManager::InteractiveShapeManager()
{
}

PNamedShape InteractiveShapeManager::GetShapeFromName(const std::string& name)
{
    ShapeIterator shapeIt = _shapeEntries.find(name);

    if (shapeIt == _shapeEntries.end())
    {
        return PNamedShape();
    }

    return shapeIt->second.shape;
}

PNamedShape InteractiveShapeManager::GetShapeFromIObject(const
        Handle_AIS_InteractiveObject obj)
{
    NameIterator it = _names.find(obj);

    if (it == _names.end())
    {
        return PNamedShape();
    }

    std::string name = it->second;

    ShapeIterator shapeIt = _shapeEntries.find(name);

    if (shapeIt == _shapeEntries.end())
    {
        // return null pointer
        return PNamedShape();
    }

    return shapeIt->second.shape;
}

std::vector<Handle(AIS_InteractiveObject)>
InteractiveShapeManager::GetIObjectsFromShapeName(const std::string& name)
{
    ShapeIterator shapeIt = _shapeEntries.find(name);

    if (shapeIt == _shapeEntries.end())
    {
        IObjectList emptyList;
        return emptyList;
    }

    return shapeIt->second.aisObjects;
}

ShapeEntry& InteractiveShapeManager::GetShapeEntry(const std::string& name)
{
    ShapeIterator shapeIt = _shapeEntries.find(name);

    if (shapeIt == _shapeEntries.end())
    {
        printf("No shape with name found");
        throw;
    }

    return shapeIt->second;
}

void InteractiveShapeManager::removeObject(const std::string& name)
{
    ShapeIterator shapeIt = _shapeEntries.find(name);

    if (shapeIt != _shapeEntries.end())
    {
        // remove mapping if iobjects with name3
        std::vector<Handle(AIS_InteractiveObject)>& ais_objects =
            shapeIt->second.aisObjects;

        for (unsigned int i = 0; i < ais_objects.size(); ++i)
        {
            NameIterator nameIt = _names.find(ais_objects[i]);

            if (nameIt != _names.end())
            {
                _names.erase(nameIt);
            }
        }

        // remove mapping of name with shape
        _shapeEntries.erase(shapeIt);
    }
}

void InteractiveShapeManager::removeObject(Handle_AIS_InteractiveObject obj)
{
    NameIterator nameIt = _names.find(obj);

    if (nameIt != _names.end())
    {
        std::string name = nameIt->second;
        _names.erase(nameIt);

        ShapeIterator shapeIt = _shapeEntries.find(name);

        if (shapeIt != _shapeEntries.end())
        {
            ShapeEntry& entry = shapeIt->second;

            // remove obj from aisObjects
            std::vector<Handle(AIS_InteractiveObject)>::iterator aisIt;
            aisIt = std::find(entry.aisObjects.begin(), entry.aisObjects.end(), obj);

            if (aisIt != entry.aisObjects.end())
            {
                entry.aisObjects.erase(aisIt);
            }

            // delete object completely, if no ais objects are in the scene
            if (entry.aisObjects.size() == 0)
            {
                _shapeEntries.erase(shapeIt);
            }
        }
    }
}

void InteractiveShapeManager::addObject(PNamedShape shape, Handle_AIS_InteractiveObject iObject)
{
    std::string name = shape->Name();

    // check if shape is already there
    ShapeIterator shapeIt = _shapeEntries.find(name);

    if (shapeIt != _shapeEntries.end())
    {
        // add iObject to shape
        ShapeEntry& entry = shapeIt->second;
        entry.aisObjects.push_back(iObject);

#ifdef DEBUG
        std::cout << "added shape " << name << std::endl;
#endif
    }
    else
    {
        // Create a new shape entry and add it to map
        ShapeEntry entry;
        entry.aisObjects.push_back(iObject);
        entry.shape     = shape;
        _shapeEntries.insert(shapeIt, ShapeMap::value_type(name, entry));

#ifdef DEBUG
        std::cout << "Created new shape " << name << std::endl;
#endif
    }

    // add also to interactive objects list
    _names[iObject] = name;
}

std::vector<PNamedShape> InteractiveShapeManager::GetAllShapes() const
{
    std::vector<PNamedShape> list;
    std::map<std::string, ShapeEntry>::const_iterator it;

    for (it = _shapeEntries.begin(); it != _shapeEntries.end(); ++it)
    {
        PNamedShape shape = it->second.shape;
        list.push_back(shape);
    }

    return list;
}
