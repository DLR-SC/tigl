/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-11-23 Martin Siggel <Martin.Siggel@dlr.de>
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


%module (package="tigl3") core

%include common.i


%{
#include "CreateIfNotExists.h"
#include "CTiglMemoryPool.h"
#include "CTiglError.h"
#include "CCPACSStringVector.h"
#include "generated/CPACSUpdates.h"
#include "generated/CPACSHeader.h"
#include "CTiglUIDObject.h"
#include "ITiglUIDRefObject.h"
%}

%feature("autodoc", "3");

%catch_exceptions()

// rename file methods to python pep8 style
%rename("%(undercase)s", %$isfunction) "";

%boost_optional(tigl::generated::CPACSUpdates)
%boost_optional(tigl::CCPACSStringVector)
%boost_optional(tigl::generated::CPACSHeader_cpacsVersion)

%include "ITiglUIDRefObject.h"
%include "CTiglUIDObject.h"
%include "CreateIfNotExists.h"
%include "CTiglMemoryPool.h"
%include "generated/CPACSStringVectorBase.h"
%include "CCPACSStringVector.h"
%include "generated/CPACSUpdates.h"
%include "generated/CPACSHeader_cpacsVersion.h"
%include "generated/CPACSHeader.h"

