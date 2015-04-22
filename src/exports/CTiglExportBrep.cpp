/* 
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-04-22 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglExportBrep.h"

// TiGL includes
#include "CCPACSConfiguration.h"
#include "CTiglLogging.h"
#include "CNamedShape.h"
#include "CTiglFusePlane.h"
#include "CCPACSWingSegment.h"
#include "CCPACSFuselageSegment.h"

// OCCT includes
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <TopoDS_Compound.hxx>

// c includes
#include <cassert>

namespace tigl
{

CTiglExportBrep::CTiglExportBrep()
{
}

CTiglExportBrep::~CTiglExportBrep()
{
}

void CTiglExportBrep::ExportBrep(CCPACSConfiguration& config, const std::string& filename) const
{
    if ( filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportBrep.";
       return;
    }

    ListPNamedShape shapes;

    // Export all wings of the configuration
    for (int w = 1; w <= config.GetWingCount(); w++) {
        CCPACSWing& wing = config.GetWing(w);

        for (int i = 1; i <= wing.GetSegmentCount(); i++) {
            CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
            PNamedShape loft = segment.GetLoft();
            shapes.push_back(loft);
        }
    }

    // Export all fuselages of the configuration
    for (int f = 1; f <= config.GetFuselageCount(); f++) {
        CCPACSFuselage& fuselage = config.GetFuselage(f);

        for (int i = 1; i <= fuselage.GetSegmentCount(); i++) {
            CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
            PNamedShape loft = segment.GetLoft();
            shapes.push_back(loft);
        }
    }

    CCPACSFarField& farfield = config.GetFarField();
    if (farfield.GetFieldType() != NONE) {
        shapes.push_back(farfield.GetLoft());
    }

    // write brep
    try {
        ExportShapes(shapes, filename);
    }
    catch (CTiglError&) {
        throw CTiglError("Cannot export airplane in CTiglExportBrep", TIGL_ERROR);
    }
}

void CTiglExportBrep::ExportFusedBrep(CCPACSConfiguration& config, const std::string& filename)
{
    if (filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportFusedBrep.";
       return;
    }

    PTiglFusePlane fuser = config.AircraftFusingAlgo();
    fuser->SetResultMode(HALF_PLANE_TRIMMED_FF);
    assert(fuser);

    PNamedShape fusedAirplane = fuser->FusedPlane();
    PNamedShape farField      = fuser->FarField();
    if (!fusedAirplane) {
        throw CTiglError("Error computing fused airplane.", TIGL_NULL_POINTER);
    }

    try {
        ListPNamedShape l;
        l.push_back(fusedAirplane);
        l.push_back(farField);

        // add intersections
        const ListPNamedShape& ints = fuser->Intersections();
        ListPNamedShape::const_iterator it;
        for (it = ints.begin(); it != ints.end(); ++it) {
            l.push_back(*it);
        }

        ExportShapes(l, filename);
    }
    catch (CTiglError&) {
        throw CTiglError("Cannot export fused Airplane as Brep", TIGL_ERROR);
    }
}

void CTiglExportBrep::ExportShapes(const ListPNamedShape& shapes, const std::string& filename) const
{
    if (shapes.size() > 1) {
        TopoDS_Compound c;
        BRep_Builder b;
        b.MakeCompound(c);
        
        for (ListPNamedShape::const_iterator it = shapes.begin(); it != shapes.end(); ++it) {
            PNamedShape shape = *it;
            if (shape) {
                b.Add(c, shape->Shape());
            }
        }
        
        // write the file
        BRepTools::Write(c, filename.c_str());
    }
    else if ( shapes.size() == 1) {
        PNamedShape shape = shapes[0];
        if (!shape) {
            LOG(WARNING) << "No shapes defined in BRep export. Abort!";
            return;
        }
        BRepTools::Write(shape->Shape(), filename.c_str());
    }
    else {
        LOG(WARNING) << "No shapes defined in BRep export. Abort!";
    }
}

} // namespace tigl
