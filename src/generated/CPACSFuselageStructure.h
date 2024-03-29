// Copyright (c) 2020 RISC Software GmbH
//
// This file was generated by CPACSGen from CPACS XML Schema (c) German Aerospace Center (DLR/SC).
// Do not edit, all changes are lost when files are re-generated.
//
// Licensed under the Apache License, Version 2.0 (the "License")
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <CCPACSCargoCrossBeamsAssembly.h>
#include <CCPACSCargoCrossBeamStrutsAssembly.h>
#include <CCPACSFramesAssembly.h>
#include <CCPACSLongFloorBeamsAssembly.h>
#include <CCPACSPressureBulkheadAssembly.h>
#include <CCPACSSkin.h>
#include <CCPACSStringersAssembly.h>
#include <CCPACSWalls.h>
#include <string>
#include <tixi.h>
#include "CPACSCargoDoorsAssembly.h"
#include "CreateIfNotExists.h"
#include "tigl_internal.h"

namespace tigl
{
class CTiglUIDManager;
class CTiglUIDObject;
class CCPACSFuselage;

namespace generated
{
    // This class is used in:
    // CPACSFuselage

    /// @brief fuselageStructureType
    /// 
    /// FuselageStructure type, containing data of the fuselage's
    /// structure
    /// 
    class CPACSFuselageStructure
    {
    public:
        TIGL_EXPORT CPACSFuselageStructure(CCPACSFuselage* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT virtual ~CPACSFuselageStructure();

        TIGL_EXPORT CCPACSFuselage* GetParent();

        TIGL_EXPORT const CCPACSFuselage* GetParent() const;

        TIGL_EXPORT virtual CTiglUIDObject* GetNextUIDParent();
        TIGL_EXPORT virtual const CTiglUIDObject* GetNextUIDParent() const;

        TIGL_EXPORT CTiglUIDManager& GetUIDManager();
        TIGL_EXPORT const CTiglUIDManager& GetUIDManager() const;

        TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;

        TIGL_EXPORT virtual const boost::optional<CCPACSSkin>& GetSkin() const;
        TIGL_EXPORT virtual boost::optional<CCPACSSkin>& GetSkin();

        TIGL_EXPORT virtual const boost::optional<CCPACSStringersAssembly>& GetStringers() const;
        TIGL_EXPORT virtual boost::optional<CCPACSStringersAssembly>& GetStringers();

        TIGL_EXPORT virtual const boost::optional<CCPACSFramesAssembly>& GetFrames() const;
        TIGL_EXPORT virtual boost::optional<CCPACSFramesAssembly>& GetFrames();

        TIGL_EXPORT virtual const boost::optional<CCPACSCargoCrossBeamsAssembly>& GetCargoCrossBeams() const;
        TIGL_EXPORT virtual boost::optional<CCPACSCargoCrossBeamsAssembly>& GetCargoCrossBeams();

        TIGL_EXPORT virtual const boost::optional<CCPACSCargoCrossBeamStrutsAssembly>& GetCargoCrossBeamStruts() const;
        TIGL_EXPORT virtual boost::optional<CCPACSCargoCrossBeamStrutsAssembly>& GetCargoCrossBeamStruts();

        TIGL_EXPORT virtual const boost::optional<CCPACSLongFloorBeamsAssembly>& GetLongFloorBeams() const;
        TIGL_EXPORT virtual boost::optional<CCPACSLongFloorBeamsAssembly>& GetLongFloorBeams();

        TIGL_EXPORT virtual const boost::optional<CCPACSPressureBulkheadAssembly>& GetPressureBulkheads() const;
        TIGL_EXPORT virtual boost::optional<CCPACSPressureBulkheadAssembly>& GetPressureBulkheads();

        TIGL_EXPORT virtual const boost::optional<CCPACSWalls>& GetWalls() const;
        TIGL_EXPORT virtual boost::optional<CCPACSWalls>& GetWalls();

        TIGL_EXPORT virtual const boost::optional<CPACSCargoDoorsAssembly>& GetCargoDoors() const;
        TIGL_EXPORT virtual boost::optional<CPACSCargoDoorsAssembly>& GetCargoDoors();

        TIGL_EXPORT virtual CCPACSSkin& GetSkin(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveSkin();

        TIGL_EXPORT virtual CCPACSStringersAssembly& GetStringers(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveStringers();

        TIGL_EXPORT virtual CCPACSFramesAssembly& GetFrames(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveFrames();

        TIGL_EXPORT virtual CCPACSCargoCrossBeamsAssembly& GetCargoCrossBeams(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveCargoCrossBeams();

        TIGL_EXPORT virtual CCPACSCargoCrossBeamStrutsAssembly& GetCargoCrossBeamStruts(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveCargoCrossBeamStruts();

        TIGL_EXPORT virtual CCPACSLongFloorBeamsAssembly& GetLongFloorBeams(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveLongFloorBeams();

        TIGL_EXPORT virtual CCPACSPressureBulkheadAssembly& GetPressureBulkheads(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemovePressureBulkheads();

        TIGL_EXPORT virtual CCPACSWalls& GetWalls(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveWalls();

        TIGL_EXPORT virtual CPACSCargoDoorsAssembly& GetCargoDoors(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveCargoDoors();

    protected:
        CCPACSFuselage* m_parent;

        CTiglUIDManager* m_uidMgr;

        boost::optional<CCPACSSkin>                         m_skin;
        boost::optional<CCPACSStringersAssembly>            m_stringers;
        boost::optional<CCPACSFramesAssembly>               m_frames;
        boost::optional<CCPACSCargoCrossBeamsAssembly>      m_cargoCrossBeams;
        boost::optional<CCPACSCargoCrossBeamStrutsAssembly> m_cargoCrossBeamStruts;
        boost::optional<CCPACSLongFloorBeamsAssembly>       m_longFloorBeams;
        boost::optional<CCPACSPressureBulkheadAssembly>     m_pressureBulkheads;
        boost::optional<CCPACSWalls>                        m_walls;
        boost::optional<CPACSCargoDoorsAssembly>            m_cargoDoors;

    private:
        CPACSFuselageStructure(const CPACSFuselageStructure&) = delete;
        CPACSFuselageStructure& operator=(const CPACSFuselageStructure&) = delete;

        CPACSFuselageStructure(CPACSFuselageStructure&&) = delete;
        CPACSFuselageStructure& operator=(CPACSFuselageStructure&&) = delete;
    };
} // namespace generated

// CPACSFuselageStructure is customized, use type CCPACSFuselageStructure directly
} // namespace tigl
