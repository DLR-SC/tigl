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

void CTiglExportBrep::AddConfiguration(CCPACSConfiguration& config)
{
    // Export all wings of the configuration
    for (int w = 1; w <= config.GetWingCount(); w++) {
        CCPACSWing& wing = config.GetWing(w);

        for (int i = 1; i <= wing.GetSegmentCount(); i++) {
            CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
            PNamedShape loft = segment.GetLoft();
            AddShape(loft);
        }
    }

    // Export all fuselages of the configuration
    for (int f = 1; f <= config.GetFuselageCount(); f++) {
        CCPACSFuselage& fuselage = config.GetFuselage(f);

        for (int i = 1; i <= fuselage.GetSegmentCount(); i++) {
            CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
            PNamedShape loft = segment.GetLoft();
            AddShape(loft);
        }
    }

    CCPACSFarField& farfield = config.GetFarField();
    if (farfield.GetFieldType() != NONE) {
        AddShape(farfield.GetLoft());
    }
}

void CTiglExportBrep::AddFusedConfiguration(CCPACSConfiguration& config)
{

    PTiglFusePlane fuser = config.AircraftFusingAlgo();
    fuser->SetResultMode(HALF_PLANE_TRIMMED_FF);
    assert(fuser);

    PNamedShape fusedAirplane = fuser->FusedPlane();
    PNamedShape farField      = fuser->FarField();
    if (!fusedAirplane) {
        throw CTiglError("Error computing fused airplane.", TIGL_NULL_POINTER);
    }

    AddShape(fusedAirplane);
    AddShape(farField);

    // add intersections
    const ListPNamedShape& ints = fuser->Intersections();
    ListPNamedShape::const_iterator it;
    for (it = ints.begin(); it != ints.end(); ++it) {
        AddShape(*it);
    }
}

void CTiglExportBrep::AddShape(PNamedShape shape)
{
    if (shape) {
        _shapes.push_back(shape);
    }
}

bool CTiglExportBrep::Write(const std::string& filename) const
{
    if (filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in CTiglExportBrep::Write.";
       return false;
    }
    
    if (_shapes.size() > 1) {
        TopoDS_Compound c;
        BRep_Builder b;
        b.MakeCompound(c);
        
        for (ListPNamedShape::const_iterator it = _shapes.begin(); it != _shapes.end(); ++it) {
            PNamedShape shape = *it;
            if (shape) {
                b.Add(c, shape->Shape());
            }
        }
        
        // write the file
        return (BRepTools::Write(c, filename.c_str()) > 0);
    }
    else if ( _shapes.size() == 1) {
        PNamedShape shape = _shapes[0];
        return (BRepTools::Write(shape->Shape(), filename.c_str()) > 0);
    }
    else {
        LOG(WARNING) << "No shapes defined in BRep export. Abort!";
        return false;
    }
}

} // namespace tigl
