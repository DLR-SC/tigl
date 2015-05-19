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
#include "CTiglError.h"

#include <STEPControl_Reader.hxx>
#include <StepBasic_ProductDefinition.hxx>
#include <StepBasic_ProductDefinitionFormation.hxx>
#include <StepBasic_Product.hxx>
#include <XSControl_WorkSession.hxx>
#include <XSControl_TransferReader.hxx>

#include <Interface_InterfaceModel.hxx>
#include <Interface_Static.hxx>
#include <IFSelect_ReturnStatus.hxx>

#include <TColStd_HSequenceOfTransient.hxx>
#include <TransferBRep.hxx>
#include <Transfer_TransientProcess.hxx>
#include <TCollection_HAsciiString.hxx>

#include <climits>

namespace
{
    void ReadShapeNames(const STEPControl_Reader& reader, ListPNamedShape& shapes) {
        // create a hash of each shape 
        typedef std::map<int, unsigned int> HashMap;
        HashMap shapeMap;
        for (unsigned int ishape = 0; ishape < shapes.size(); ++ishape) {
            PNamedShape shape = shapes[ishape];
            if (!shape) {
                continue;
            }

            int hash = shape->Shape().HashCode(INT_MAX);
            shapeMap[hash] = ishape;
        }

        Handle(Interface_InterfaceModel) model = reader.Model();
        Handle(Transfer_TransientProcess) process = reader.WS()->TransferReader()->TransientProcess();

        for (int iEnt = 1; iEnt <= model->NbEntities(); iEnt ++) {
            Handle(Standard_Transient) stepEntity = model->Value(iEnt);

            // Retrieve the shape name from the step product name
            if (stepEntity->IsKind ( STANDARD_TYPE(StepBasic_ProductDefinition ))) {
                Handle(StepBasic_ProductDefinition) pd = Handle(StepBasic_ProductDefinition)::DownCast(stepEntity);
                Standard_Integer mapIndex = process->MapIndex(pd);

                if (mapIndex <= 0) {
                    continue;
                }

                // get the shape
                Handle(Transfer_Binder) binder = process->MapItem (mapIndex);
                TopoDS_Shape boundShape = TransferBRep::ShapeResult(binder);
                if ( boundShape.IsNull() ) {
                    continue;
                }

                // get the product name
                Handle(StepBasic_Product) prod = pd->Formation()->OfProduct();
                if (prod.IsNull()) {
                    continue;
                }

                std::string shapeName = prod->Name()->ToCString();

                // create hash and search for it in hashMap
                int hash = boundShape.HashCode(INT_MAX);
                HashMap::iterator it = shapeMap.find(hash);
                if (it == shapeMap.end()) {
                    continue;
                }

                unsigned int shapeIndex = it->second;
                PNamedShape theShape = shapes[shapeMap[shapeIndex]];
                theShape->SetName(shapeName.c_str());
                theShape->SetShortName(shapeName.c_str());
            }
        }
    } // read shape names
}

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
    Interface_Static::SetCVal("xstep.cascade.unit", "M");
    IFSelect_ReturnStatus status = aReader.ReadFile(stepFileName.c_str());
    if ( status == IFSelect_RetDone ) {
        int nbr = aReader.NbRootsForTransfer();
        for ( Standard_Integer n = 1; n <= nbr; n++ ) {
            aReader.TransferRoot( n );
        }
    }
    else {
        throw CTiglError( "Cannot read step file " + stepFileName + "!", TIGL_OPEN_FAILED);
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

    ReadShapeNames(aReader, shapeList);

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

