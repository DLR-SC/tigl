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
#include "CTiglStepReader.h"

#include "CTiglLogging.h"
#include "CNamedShape.h"

#include <STEPControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>

namespace tigl
{

CTiglStepReader::CTiglStepReader()
{
}

PNamedShape CTiglStepReader::read(const std::string stepFileName)
{
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
        return PNamedShape();
    }

    int nbs = aReader.NbShapes();
    if ( nbs > 0 ) {
        PNamedShape pshape(new CNamedShape(aReader.OneShape(), "StepImport", "StepFile"));
        return pshape;
    }
    else {
        LOG(ERROR) << "No shapes could be read from step file " << stepFileName << "!";
        return PNamedShape();
    }
}

CTiglStepReader::~CTiglStepReader()
{
}

}

