/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-05-10 Martin Siggel <martin.siggel@dlr.de>
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


#ifndef CADIMPORT_H_
#define CADIMPORT_H_

#include <Handle_TopTools_HSequenceOfShape.hxx>
#include <ListPNamedShape.h>
#include "OsgMainApp.hpp"

void openCADModel( const FileFormat format, const std::string& file );
void importSTEP ( const std::string& file );
void importIGES ( const std::string& file );
void importBREP( const std::string& file );
void importCPACS( const std::string& file );

#endif // CADIMPORT_H_
