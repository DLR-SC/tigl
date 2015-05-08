/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-05 Martin Siggel <Martin.Siggel@dlr.de>
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
#include "CTiglStepReader.h"

#include "CTiglLogging.h"
#include "CNamedShape.h"
#include "ICADImporterCreator.h"
#include "CTiglImporterFactory.h"
#include "CTiglTypeRegistry.h"


#include <STEPControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>

namespace tigl
{

// register at factory

AUTORUN(CTiglStepReader)
{
    static ICADImporterCreatorImpl<CTiglStepReader> stepImporterCreator;
    CTiglImporterFactory::Instance().RegisterImporter(&stepImporterCreator);
    return true;
}

CTiglStepReader::CTiglStepReader()
{
}

ListPNamedShape CTiglStepReader::Read(const std::string stepFileName)
{
    ListPNamedShape shapeList;

    STEPControl_Reader aReader;
    IFSelect_ReturnStatus status = aReader.ReadFile(stepFileName.c_str());
    if ( status == IFSelect_RetDone ) {
        int nbr = aReader.NbRootsForTransfer();
        for ( Standard_Integer n = 1; n <= nbr; n++ ) {
            aReader.TransferRoot( n );
        }
    }
    else {
        LOG(ERROR) << "Error reading in step file " << stepFileName << "!";
        return shapeList;
    }

    int nbs = aReader.NbShapes();
    if ( nbs == 0 ) {
        LOG(WARNING) << "No shapes could be found in step file " << stepFileName << "!";
    }
    for (int ishape = 1; ishape <= nbs; ++ishape) {
        std::stringstream shapeName, shapeShortName;
        shapeName << "StepImport_" << ishape;
        shapeShortName << "STEP" << ishape;

        PNamedShape pshape(new CNamedShape(aReader.Shape(ishape), shapeName.str().c_str(), shapeShortName.str().c_str()));
        shapeList.push_back(pshape);
    }

    return shapeList;
}

CTiglStepReader::~CTiglStepReader()
{
}

std::string tigl::CTiglStepReader::SupportedFileType() const
{
    return "step";
}

}

