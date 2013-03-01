/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
/**
* @file
* @brief  Export routines for CPACS configurations.
*/

// standard libraries
#include <iostream>

#include "CTiglLogger.h"
#include "CTiglExportVtk.h"
#include "CCPACSConfiguration.h"
#include "CTiglAbstractPhysicalComponent.h"
#include "CTiglAbstractSegment.h"
#include "CCPACSWing.h"
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSWingSegment.h"


// algorithms
#include "BRepMesh.hxx"


#include "CTiglPolyData.h"
#include "CTiglTriangularizer.h"

namespace tigl {

    // Constructor
    CTiglExportVtk::CTiglExportVtk(CCPACSConfiguration& config)
        :myConfig(config)
    {
    }

    // Destructor
    CTiglExportVtk::~CTiglExportVtk(void)
    {
    }
	
    
    // Exports a by index selected wing, boolean fused and meshed, as STL file
    void CTiglExportVtk::ExportMeshedWingVTKByIndex(const int wingIndex, const std::string& filename, const double deflection)
    {
        const std::string& wingUID = myConfig.GetWing(wingIndex).GetUID();
        ExportMeshedWingVTKByUID(wingUID, filename, deflection);
    }

    // Exports a by UID selected wing, boolean fused and meshed, as STL file
    void CTiglExportVtk::ExportMeshedWingVTKByUID(const std::string wingUID, const std::string& filename, const double deflection)
    {
        tigl::CCPACSWing& wing = myConfig.GetWing(wingUID);
        BRepMesh::Mesh(wing.GetLoft(), deflection);
        
        CTiglTriangularizer wingTrian(wing, SEGMENT_INFO);
        wingTrian.writeVTK(filename.c_str());
    }



    // Exports a by index selected fuselage, boolean fused and meshed, as VTK file
    void CTiglExportVtk::ExportMeshedFuselageVTKByIndex(const int fuselageIndex, const std::string& filename, const double deflection)
    {
        CTiglAbstractPhysicalComponent & component = myConfig.GetFuselage(fuselageIndex);
        TopoDS_Shape loft = component.GetLoft();

        LOG(ERROR) << "tiglExportMeshedFuselageByIndexVTK not yet implemented!!" << std::endl;
    }

    // Exports a by UID selected fuselage, boolean fused and meshed, as VTK file
    void CTiglExportVtk::ExportMeshedFuselageVTKByUID(const std::string fuselageUID, const std::string& filename, const double deflection)
    {
        CTiglAbstractPhysicalComponent & component = myConfig.GetFuselage(fuselageUID);
        TopoDS_Shape loft = component.GetLoft();
        
        LOG(ERROR) << "tiglExportMeshedFuselageByIndexUID not yet implemented!!" << std::endl;
    }
    

    // Exports a whole geometry, boolean fused and meshed, as VTK file
    void CTiglExportVtk::ExportMeshedGeometryVTK(const std::string& filename, const double deflection)
    {
        LOG(ERROR) << "tiglExportMeshedGeometryVTK not yet implemented!!" << std::endl;
    }

    /************* Simple ones *************************/
    // Exports a by UID selected wing, boolean fused and meshed, as STL file
    void CTiglExportVtk::ExportMeshedWingVTKSimpleByUID(const std::string wingUID, const std::string& filename, const double deflection)
    {
        CTiglAbstractPhysicalComponent & component = myConfig.GetWing(wingUID);
        TopoDS_Shape& loft = component.GetLoft();
        BRepMesh::Mesh(loft, deflection);
        
        CTiglTriangularizer loftTrian(loft);
        loftTrian.writeVTK(filename.c_str());
    }
    
    /************* Simple ones *************************/
    // Exports a by UID selected wing, boolean fused and meshed, as STL file
    void CTiglExportVtk::ExportMeshedWingVTKSimpleByIndex(const int wingIndex, const std::string& filename, const double deflection)
    {
        const std::string& wingUID = myConfig.GetWing(wingIndex).GetUID();
        ExportMeshedWingVTKSimpleByUID(wingUID, filename, deflection);
    }

    // Exports a by UID selected fuselage, boolean fused and meshed, as VTK file
    void CTiglExportVtk::ExportMeshedFuselageVTKSimpleByUID(const std::string fuselageUID, const std::string& filename, const double deflection)
    {
        CTiglAbstractPhysicalComponent & component = myConfig.GetFuselage(fuselageUID);
        TopoDS_Shape& loft = component.GetLoft();
        BRepMesh::Mesh(loft, deflection);
        
        CTiglTriangularizer loftTrian(loft);
        loftTrian.writeVTK(filename.c_str());
    }
    
    // Exports a by UID selected fuselage, boolean fused and meshed, as VTK file
    void CTiglExportVtk::ExportMeshedFuselageVTKSimpleByIndex(const int fuselageIndex, const std::string& filename, const double deflection){
        const std::string& fuselageUID = myConfig.GetFuselage(fuselageIndex).GetUID();
        ExportMeshedFuselageVTKSimpleByUID(fuselageUID, filename, deflection);
    }


    // Exports a whole geometry, boolean fused and meshed, as VTK file
    void CTiglExportVtk::ExportMeshedGeometryVTKSimple(const std::string& filename, const double deflection)
    {

        LOG(ERROR) << "tiglExportMeshedGeometryVTKSimple not yet implemented!!" << std::endl;
    }

} // end namespace tigl
