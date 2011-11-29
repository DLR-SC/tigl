/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
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
* @brief  Implementation of CPACS configuration handling routines.
*/

#include "CCPACSConfiguration.h"
#include "TopoDS_Shape.hxx"
#include "Standard_CString.hxx"
#include "BRepOffsetAPI_ThruSections.hxx"
#include "BRepAlgoAPI_Fuse.hxx"
#include "BRepAlgo_Fuse.hxx"
#include "ShapeFix_Shape.hxx"
#include "TopoDS_Compound.hxx"
#include "BRepFeat_Gluer.hxx"
#include "BRep_Builder.hxx"
#include "BRepMesh.hxx"
#include "IGESControl_Controller.hxx"
#include "IGESControl_Writer.hxx"
#include "StlAPI_Writer.hxx"
#include "Interface_Static.hxx"
#include "StlAPI.hxx"


namespace tigl {

	// Constructor
	CCPACSConfiguration::CCPACSConfiguration(TixiDocumentHandle tixiHandle)
		: tixiDocumentHandle(tixiHandle)
		, header()
		, wings(this)
        , fuselages(this)
        , uidManager()
	{
	}

	// Destructor
	CCPACSConfiguration::~CCPACSConfiguration(void)
	{
	}

	// Invalidates the internal state of the configuration and forces
	// recalculation of wires, lofts etc.
	void CCPACSConfiguration::Invalidate(void)
	{
		wings.Invalidate();
        fuselages.Invalidate();
	}

	// Build up memory structure for whole CPACS file
	void CCPACSConfiguration::ReadCPACS(char* configurationUID)
	{
		header.ReadCPACS(tixiDocumentHandle);
		wings.ReadCPACS(tixiDocumentHandle, configurationUID);
        fuselages.ReadCPACS(tixiDocumentHandle, configurationUID);
	}


	// Returns the boolean fused airplane as TopoDS_Shape
	TopoDS_Shape CCPACSConfiguration::GetFusedAirplane(void)
    {
		ITiglGeometricComponent* rootComponent = uidManager.GetRootComponent();
		fusedAirplane = rootComponent->GetLoft();
		OutputComponentTree(rootComponent);
		return(fusedAirplane);
	}


	// This function does the boolean fusing 
    void CCPACSConfiguration::OutputComponentTree(ITiglGeometricComponent* parent)
    {
        ITiglGeometricComponent::ChildContainerType& children = parent->GetChildren();
        ITiglGeometricComponent::ChildContainerType::iterator pIter;
        for (pIter = children.begin(); pIter != children.end(); pIter++) {
            ITiglGeometricComponent* child = *pIter;
			TopoDS_Shape tmpShape = child->GetLoft();
			fusedAirplane = BRepAlgoAPI_Fuse(fusedAirplane, tmpShape);
            OutputComponentTree(child);
        }
    }

	// Returns the underlying tixi document handle used by a CPACS configuration
	TixiDocumentHandle CCPACSConfiguration::GetTixiDocumentHandle(void) const
	{
		return tixiDocumentHandle;
	}

	// Returns the total count of wing profiles in this configuration
	int CCPACSConfiguration::GetWingProfileCount(void) const
	{
		return wings.GetProfileCount();
	}

	// Returns the wing profile for a given uid.
	CCPACSWingProfile& CCPACSConfiguration::GetWingProfile(std::string uid) const
	{
		return wings.GetProfile(uid);
	}

	// Returns the wing profile for a given index - TODO: depricated function!
	CCPACSWingProfile& CCPACSConfiguration::GetWingProfile(int index) const
	{
		return wings.GetProfile(index);
	}

	// Returns the total count of wings in a configuration
	int CCPACSConfiguration::GetWingCount(void) const
	{
		return wings.GetWingCount();
	}

	// Returns the wing for a given index.
	CCPACSWing& CCPACSConfiguration::GetWing(int index) const
	{
		return wings.GetWing(index);
	}
    // Returns the wing for a given UID.
    CCPACSWing& CCPACSConfiguration::GetWing(const std::string UID) const
    {
        return wings.GetWing(UID);
    }

	// Returns the total count of fuselage profiles in this configuration
	int CCPACSConfiguration::GetFuselageProfileCount(void) const
	{
		return fuselages.GetProfileCount();
	}

	// Returns the fuselage profile for a given index.
	CCPACSFuselageProfile& CCPACSConfiguration::GetFuselageProfile(int index) const
	{
		return fuselages.GetProfile(index);
	}

	// Returns the fuselage profile for a given uid.
	CCPACSFuselageProfile& CCPACSConfiguration::GetFuselageProfile(std::string uid) const
	{
		return fuselages.GetProfile(uid);
	}

	// Returns the total count of fuselages in a configuration
	int CCPACSConfiguration::GetFuselageCount(void) const
	{
		return fuselages.GetFuselageCount();
	}

	// Returns the fuselage for a given index.
	CCPACSFuselage& CCPACSConfiguration::GetFuselage(int index) const
	{
		return fuselages.GetFuselage(index);
	}

    // Returns the fuselage for a given UID.
    CCPACSFuselage& CCPACSConfiguration::GetFuselage(const std::string UID) const
    {
        return fuselages.GetFuselage(UID);
    }

    // Returns the uid manager
    CTiglUIDManager& CCPACSConfiguration::GetUIDManager(void)
    {
        return uidManager;
    }

} // end namespace tigl
