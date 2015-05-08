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
#include "CTiglImporterFactory.h"

TEST(TiglImport, Step)
{
    tigl::CTiglStepReader reader;
    ListPNamedShape shapes = reader.Read("TestData/nacelle.stp");
    ASSERT_EQ(1, shapes.size());
}

TEST(TiglImport, ImporterFactory)
{
    tigl::CTiglImporterFactory factory = tigl::CTiglImporterFactory::Instance();

    tigl::ITiglCADImporter* importer = NULL;
    importer = factory.Create("step");
    ASSERT_TRUE(importer != NULL);
    ASSERT_STREQ("step", importer->SupportedFileType().c_str());
    delete importer;

    importer = factory.Create("STEP");
    ASSERT_TRUE(importer != NULL);
    ASSERT_STREQ("step", importer->SupportedFileType().c_str());
    delete importer;

    importer = factory.Create("invalidformat");
    ASSERT_TRUE(importer == NULL);
    
    ASSERT_TRUE(factory.ImporterSupported("step"));
    ASSERT_FALSE(factory.ImporterSupported("invalidformat"));
}
