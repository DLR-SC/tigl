/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-08 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef ICADIMPORTERCREATOR_H
#define ICADIMPORTERCREATOR_H

#include "ITiglCADImporter.h"

namespace tigl
{

class ICADImporterCreator
{
public:
   virtual PTiglCADImporter create() const = 0;
};

template <class T>
class ICADImporterCreatorImpl : public ICADImporterCreator
{
public:
   PTiglCADImporter create() const
   {
       return PTiglCADImporter(new T);
   }
};

}

#endif // ICADIMPORTERCREATOR_H
