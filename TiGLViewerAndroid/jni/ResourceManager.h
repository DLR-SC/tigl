/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
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

#ifndef RESOURCEMANAGER_H_
#define RESOURCEMANAGER_H_

#include <istream>
#include <osg/ref_ptr>
#include <osgText/Font>

class ResourceManager
{
public:
    ResourceManager();

    static osg::ref_ptr<osgText::Font> OpenFontFile(const char * filename);

    virtual ~ResourceManager();
};

#endif /* RESOURCEMANAGER_H_ */
