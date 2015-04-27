/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-27 Martin Siggel <Martin.Siggel@dlr.de>
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


#include "test.h"
#include "CTiglStepReader.h"
#include "CTiglExportStep.h"
#include "CNamedShape.h"
#include "CCPACSConfiguration.h"

TEST(TiglImport, Step)
{
    tigl::CTiglStepReader reader;
    PNamedShape shape = reader.read("TestData/nacelle.stp");
    ASSERT_TRUE(shape != NULL);
    shape->SetName("Nacelle");
    tigl::CCPACSConfiguration config(0);
    
    ListPNamedShape list;
    list.push_back(shape);
    tigl::CTiglExportStep writer(config);
    writer.ExportShapes(list, "TestData/export/nacelle-out.stp");
}
