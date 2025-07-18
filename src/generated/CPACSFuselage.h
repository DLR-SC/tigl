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
#include <CCPACSFuselageSegments.h>
#include <CCPACSFuselageStructure.h>
#include <CCPACSPositionings.h>
#include <CCPACSTransformation.h>
#include <string>
#include <TiglSymmetryAxis.h>
#include <tixi.h>
#include "CPACSCompartments.h"
#include "CPACSFuselageSections.h"
#include "CreateIfNotExists.h"
#include "CTiglUIDObject.h"
#include "ITiglUIDRefObject.h"
#include "tigl_internal.h"

namespace tigl
{
class CTiglUIDManager;

namespace generated
{
    class CPACSFuselages;

    // This class is used in:
    // CPACSFuselages

    /// @brief fuselageType
    /// 
    /// Fuselage type, containing all data related to a
    /// fuselage
    /// @see geomBlock
    /// 
    class CPACSFuselage : public CTiglReqUIDObject, public ITiglUIDRefObject
    {
    public:
        TIGL_EXPORT CPACSFuselage(CPACSFuselages* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT virtual ~CPACSFuselage();

        TIGL_EXPORT CPACSFuselages* GetParent();

        TIGL_EXPORT const CPACSFuselages* GetParent() const;

        TIGL_EXPORT virtual CTiglUIDObject* GetNextUIDParent();
        TIGL_EXPORT virtual const CTiglUIDObject* GetNextUIDParent() const;

        TIGL_EXPORT CTiglUIDManager& GetUIDManager();
        TIGL_EXPORT const CTiglUIDManager& GetUIDManager() const;

        TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;

        TIGL_EXPORT virtual const std::string& GetUID() const;
        TIGL_EXPORT virtual void SetUID(const std::string& value);

        TIGL_EXPORT virtual const boost::optional<TiglSymmetryAxis>& GetSymmetry() const;
        TIGL_EXPORT virtual void SetSymmetry(const boost::optional<TiglSymmetryAxis>& value);

        TIGL_EXPORT virtual const std::string& GetName() const;
        TIGL_EXPORT virtual void SetName(const std::string& value);

        TIGL_EXPORT virtual const boost::optional<std::string>& GetDescription() const;
        TIGL_EXPORT virtual void SetDescription(const boost::optional<std::string>& value);

        TIGL_EXPORT virtual const boost::optional<std::string>& GetParentUID() const;
        TIGL_EXPORT virtual void SetParentUID(const boost::optional<std::string>& value);

        TIGL_EXPORT virtual const CCPACSTransformation& GetTransformation() const;
        TIGL_EXPORT virtual CCPACSTransformation& GetTransformation();

        TIGL_EXPORT virtual const CPACSFuselageSections& GetSections() const;
        TIGL_EXPORT virtual CPACSFuselageSections& GetSections();

        TIGL_EXPORT virtual const boost::optional<CCPACSPositionings>& GetPositionings() const;
        TIGL_EXPORT virtual boost::optional<CCPACSPositionings>& GetPositionings();

        TIGL_EXPORT virtual const CCPACSFuselageSegments& GetSegments() const;
        TIGL_EXPORT virtual CCPACSFuselageSegments& GetSegments();

        TIGL_EXPORT virtual const boost::optional<CCPACSFuselageStructure>& GetStructure() const;
        TIGL_EXPORT virtual boost::optional<CCPACSFuselageStructure>& GetStructure();

        TIGL_EXPORT virtual const boost::optional<CPACSCompartments>& GetCompartments() const;
        TIGL_EXPORT virtual boost::optional<CPACSCompartments>& GetCompartments();

        TIGL_EXPORT virtual CCPACSPositionings& GetPositionings(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemovePositionings();

        TIGL_EXPORT virtual CCPACSFuselageStructure& GetStructure(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveStructure();

        TIGL_EXPORT virtual CPACSCompartments& GetCompartments(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveCompartments();

    protected:
        CPACSFuselages* m_parent;

        CTiglUIDManager* m_uidMgr;

        std::string                              m_uID;

        boost::optional<TiglSymmetryAxis>        m_symmetry;

        /// Name of fuselage
        std::string                              m_name;

        /// Description of fuselage
        boost::optional<std::string>             m_description;

        /// UID of part to which the fuselage is
        /// mounted (if any)
        boost::optional<std::string>             m_parentUID;

        CCPACSTransformation                     m_transformation;

        CPACSFuselageSections                    m_sections;

        boost::optional<CCPACSPositionings>      m_positionings;

        CCPACSFuselageSegments                   m_segments;

        boost::optional<CCPACSFuselageStructure> m_structure;

        boost::optional<CPACSCompartments>       m_compartments;

    private:
        TIGL_EXPORT const CTiglUIDObject* GetNextUIDObject() const final;
        TIGL_EXPORT void NotifyUIDChange(const std::string& oldUid, const std::string& newUid) final;

        CPACSFuselage(const CPACSFuselage&) = delete;
        CPACSFuselage& operator=(const CPACSFuselage&) = delete;

        CPACSFuselage(CPACSFuselage&&) = delete;
        CPACSFuselage& operator=(CPACSFuselage&&) = delete;
    };
} // namespace generated

// CPACSFuselage is customized, use type CCPACSFuselage directly

// Aliases in tigl namespace
using CCPACSFuselages = generated::CPACSFuselages;
} // namespace tigl
