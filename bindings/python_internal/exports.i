/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-11-30 Martin Siggel <Martin.Siggel@dlr.de>
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

%module (package="tigl") exports

%include common.i

%import core.i
%import geometry.i


%{
#include "CNamedShape.h"
#include "CTiglExportBrep.h"
#include "CTiglExportStep.h"
#include "CTiglExportIges.h"
#include "CTiglExportStl.h"
#include "CTiglExportVtk.h"
#include "CTiglExportCollada.h"
#include "CCPACSImportExport.h"
%}

%template(ListPNamedShape) std::vector<PNamedShape>;

%feature("autodoc", "3");

// rename file methods to python pep8 style
%rename("%(undercase)s", %$isfunction) "";


%include "CCPACSImportExport.h"
%include "CTiglCADExporter.h"
%include "CTiglExportBrep.h"
%include "CTiglExportStep.h"
%include "CTiglExportIges.h"
%include "CTiglExportStl.h"
%include "CTiglExportVtk.h"
%include "CTiglExportCollada.h"
