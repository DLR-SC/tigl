/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-15 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CTIGLTYPEREGISTRY_H
#define CTIGLTYPEREGISTRY_H

/**
 * The registry allows classes to enrigster class functions, that
 * should be run automatically after loading TiGL.
 * This mechanism can be used e.g. to register classes at class factories.
 * 
 * In order to make use of the mechanism, implement an AUTOSTART function inside the
 * classes cpp file i.e.:
 * 
 * MyClass.cpp:
 * 
 *    AUTORUN(MyClass)
 *    {
 *        // your code here
 *        return true;
 *    }
 * 
 * Then, this autorun function has to be registered inside the registry. This is done
 * by placing
 *    REGISTER_TYPE(MyClass)
 * inside CTiglTypeRegistry.cpp.
 */

namespace tigl
{

class CTiglTypeRegistry
{
public:
    static void Init();
};

}

#define AUTORUN(type) \
    bool type ## _RegisterType()

#endif // CTIGLTYPEREGISTRY_H
