/* 
* Copyright (C) 2012 Cassidian Air Systems
*
* Created: 2012-11-01 Roland Landertshamer
*
* Changed: $Id $ 
*
* Version: $Revision $
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
// [[CAS_AES]]
// Added class CTiglIGESWriter for exporting model in IGES format with 
// additional information:
//  - level: defines the IGES level for the exported geometry
//
// The basic functionality is taken from IGESControl_Writer.cxx from the
// OpenCASCADE distribution.

#ifndef CTIGLIGESWRITER_HXX
#define CTIGLIGESWRITER_HXX

#include <Handle_Transfer_FinderProcess.hxx>
#include <Handle_IGESData_IGESModel.hxx>
#include <IGESData_BasicEditor.hxx>
#include <Standard_Boolean.hxx>
#include <TopoDS_Shape.hxx>

class CTiglIGESWriter
{
public:

    CTiglIGESWriter();

    Standard_Boolean AddShape(const TopoDS_Shape& sh, int level);

    void ComputeModel();

    Standard_Boolean Write(const Standard_CString file);

protected:
    void assignLevelToTrimmedSurfaces(Handle(IGESData_IGESEntity) ent, int level);
    void assignLevelToAllEntities(Handle(IGESData_IGESEntity) ent, int level);

protected:

    Handle_Transfer_FinderProcess transferFinderProcess;
    Handle_IGESData_IGESModel igesModel;
    IGESData_BasicEditor editor;
    Standard_Integer writeMode;
    Standard_Boolean isValid;
};

#endif // CTIGLIGESWRITER_HXX
