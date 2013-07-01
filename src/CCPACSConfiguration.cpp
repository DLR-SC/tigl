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
#include "BRepTools.hxx"
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>

#include <cfloat>

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
        fusedAirplane.Nullify();
        shapeCache.Clear();
        configUID = "";
    }

    // Build up memory structure for whole CPACS file
    void CCPACSConfiguration::ReadCPACS(const char* configurationUID)
    {
        if(!configurationUID) return;

        header.ReadCPACS(tixiDocumentHandle);
        wings.ReadCPACS(tixiDocumentHandle, configurationUID);
        fuselages.ReadCPACS(tixiDocumentHandle, configurationUID);

        configUID = configurationUID;
        // Now do parent <-> child transformations. Child should use the
        // parent coordinate system as root. 
        try {
            transformAllComponents(uidManager.GetRootComponent());    
        }
        catch (tigl::CTiglError& ex) {
            LOG(ERROR) << ex.getError() << std::endl;
        }
    }

    // transform all components relative to their parents
    void CCPACSConfiguration::transformAllComponents(CTiglAbstractPhysicalComponent* parent)
    {
        CTiglAbstractPhysicalComponent::ChildContainerType children = parent->GetChildren(false);
        CTiglAbstractPhysicalComponent::ChildContainerType::iterator pIter;
        CTiglPoint parentTranslation = parent->GetTranslation();
        for (pIter = children.begin(); pIter != children.end(); ++pIter) {
            CTiglAbstractPhysicalComponent* child = *pIter;
            child->Translate(parentTranslation);
            transformAllComponents(child);
            
        }
    }


    // Returns the boolean fused airplane as TopoDS_Shape
    TopoDS_Shape& CCPACSConfiguration::GetFusedAirplane(void)
    {
        if(fusedAirplane.IsNull()){
            CTiglAbstractPhysicalComponent* rootComponent = uidManager.GetRootComponent();
            BuildFusedPlane(rootComponent);
        }
        return(fusedAirplane);
    }


    // This function does the boolean fusing 
    void CCPACSConfiguration::BuildFusedPlane(CTiglAbstractPhysicalComponent *parent)
    {
        if(!parent)
            throw CTiglError("Null Pointer argument in CCPACSConfiguration::OutputComponentTree", TIGL_NULL_POINTER);

        bool calcFullModel = true;
        
        CTiglAbstractPhysicalComponent::ChildContainerType children = parent->GetChildren(false);
        
        // count number of fusing operations
        int fuseCount = 0, ifuse = 0;
        CTiglAbstractPhysicalComponent::ChildContainerType::iterator pIter;
        for (pIter = children.begin(); pIter != children.end(); ++pIter) {
            CTiglAbstractPhysicalComponent* child = *pIter;
            if(child->GetSymmetryAxis()!= TIGL_NO_SYMMETRY && calcFullModel)
                fuseCount++;
            if(pIter != children.begin())
                fuseCount++;
        }
        if(parent->GetSymmetryAxis() != TIGL_NO_SYMMETRY && calcFullModel && (parent->GetComponentType()& TIGL_COMPONENT_WING))
            fuseCount++;
        // parent with childs
        if(!children.empty())
            fuseCount++;
        
        // this somewhat complicated fusing is the only way, that the result of all tested planes was okay
        TopoDS_Shape fusedChilds;
        for (pIter = children.begin(); pIter != children.end(); ++pIter) {
            CTiglAbstractPhysicalComponent* child = *pIter;
            TopoDS_Shape fusedChild = child->GetLoft();
            TopoDS_Shape tmpShape = child->GetMirroredLoft();
            if(!tmpShape.IsNull() && calcFullModel) {
                try{
                    fusedChild = BRepAlgoAPI_Fuse(fusedChild, tmpShape);
                    LOG(INFO) << (++ifuse)/(double)fuseCount * 100. << "% ";
                }
                catch(...){
                    throw CTiglError( "Error fusing "  + child->GetUID() + " with mirrored component ", TIGL_ERROR);
                }
            }
            if(!fusedChilds.IsNull()) {
                try{
                    fusedChilds = BRepAlgoAPI_Fuse(fusedChilds, fusedChild);
                    LOG(INFO) << (++ifuse)/(double)fuseCount * 100. << "% ";
                }
                catch(...){
                    throw CTiglError( "Error fusing component"  + child->GetUID() + " with plane ", TIGL_ERROR);
                }
            }
            else
                fusedChilds = fusedChild;
        }
        
        //create root component
        TopoDS_Shape parentShape = parent->GetLoft();
        TopoDS_Shape tmpShape = parent->GetMirroredLoft();
        if(!tmpShape.IsNull() && calcFullModel && (parent->GetComponentType()& TIGL_COMPONENT_WING)){
            try{
                parentShape = BRepAlgoAPI_Fuse(parentShape, tmpShape);
                LOG(INFO) << (++ifuse)/(double)fuseCount * 100. << "% ";
            }
            catch(...){
                throw CTiglError( "Error fusing "  + parent->GetUID() + " mirrored component!", TIGL_ERROR);
            }
        }
        
        // fuse childs with root
        if(!fusedChilds.IsNull()){
            try{
                fusedAirplane = BRepAlgoAPI_Fuse(parentShape, fusedChilds);
                LOG(INFO) << (++ifuse)/(double)fuseCount * 100. << "% ";
            }
            catch(...){
                throw CTiglError( "Error fusing parent loft \""  + parent->GetUID() + "\" with its childs!", TIGL_ERROR);
            }
        }
        else
            fusedAirplane = parentShape;
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
    CCPACSWing& CCPACSConfiguration::GetWing(const std::string& UID) const
    {
        return wings.GetWing(UID);
    }

    TopoDS_Shape CCPACSConfiguration::GetParentLoft(const std::string& UID)
    {
        return uidManager.GetParentComponent(UID)->GetLoft();
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

    double CCPACSConfiguration::GetAirplaneLenth(void){
        Bnd_Box boundingBox;

        // Draw all wings
        for (int w = 1; w <= GetWingCount(); w++)
        {
            tigl::CCPACSWing& wing = GetWing(w);

            for (int i = 1; i <= wing.GetSegmentCount(); i++)
            {
                tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
                BRepBndLib::Add(segment.GetLoft(), boundingBox);

            }

            if(wing.GetSymmetryAxis() == TIGL_NO_SYMMETRY)
                continue;

            for (int i = 1; i <= wing.GetSegmentCount(); i++)
            {
                tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
                BRepBndLib::Add(segment.GetLoft(), boundingBox);
            }
        }

        for (int f = 1; f <= GetFuselageCount(); f++)
        {
            tigl::CCPACSFuselage& fuselage = GetFuselage(f);

            for (int i = 1; i <= fuselage.GetSegmentCount(); i++)
            {
                tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
                TopoDS_Shape loft = segment.GetLoft();

                // Transform by fuselage transformation
                loft = fuselage.GetFuselageTransformation().Transform(loft);
                BRepBndLib::Add(segment.GetLoft(), boundingBox);
            }
        }
        Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
        boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        return xmax-xmin;
    }

    // Returns the uid manager
    std::string CCPACSConfiguration::GetUID(void)
    {
        return configUID;
    }
    
    CTiglShapeCache& CCPACSConfiguration::GetShapeCache() {
        return shapeCache;
    }

} // end namespace tigl
