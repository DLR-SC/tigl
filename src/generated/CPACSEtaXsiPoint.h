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

#include <string>
#include <tixi.h>
#include <typeinfo>
#include "CTiglError.h"
#include "ITiglUIDRefObject.h"
#include "tigl_internal.h"

namespace tigl
{
class CTiglUIDManager;
class CTiglUIDObject;
class CCPACSWingSparPosition;
class CCPACSWingRibExplicitPositioning;
class CCPACSWingRibsPositioning;

namespace generated
{
    // This class is used in:
    // CPACSSparPosition
    // CPACSWingRibExplicitPositioning
    // CPACSWingRibsPositioning

    /// @brief Point in eta and xsi coordinates
    /// 
    /// Point described by eta-xsi coordinates.
    /// Can be either segment or component segment coordinates.
    /// 
    class CPACSEtaXsiPoint : public ITiglUIDRefObject
    {
    public:
        TIGL_EXPORT CPACSEtaXsiPoint(CCPACSWingSparPosition* parent, CTiglUIDManager* uidMgr);
        TIGL_EXPORT CPACSEtaXsiPoint(CCPACSWingRibExplicitPositioning* parent, CTiglUIDManager* uidMgr);
        TIGL_EXPORT CPACSEtaXsiPoint(CCPACSWingRibsPositioning* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT virtual ~CPACSEtaXsiPoint();

        template<typename P>
        bool IsParent() const
        {
            return m_parentType != NULL && *m_parentType == typeid(P);
        }

        template<typename P>
        P* GetParent()
        {
            static_assert(std::is_same<P, CCPACSWingSparPosition>::value || std::is_same<P, CCPACSWingRibExplicitPositioning>::value || std::is_same<P, CCPACSWingRibsPositioning>::value, "template argument for P is not a parent class of CPACSEtaXsiPoint");
            if (!IsParent<P>()) {
                throw CTiglError("bad parent");
            }
            return static_cast<P*>(m_parent);
        }

        template<typename P>
        const P* GetParent() const
        {
            static_assert(std::is_same<P, CCPACSWingSparPosition>::value || std::is_same<P, CCPACSWingRibExplicitPositioning>::value || std::is_same<P, CCPACSWingRibsPositioning>::value, "template argument for P is not a parent class of CPACSEtaXsiPoint");
            if (!IsParent<P>()) {
                throw CTiglError("bad parent");
            }
            return static_cast<P*>(m_parent);
        }

        TIGL_EXPORT virtual CTiglUIDObject* GetNextUIDParent();
        TIGL_EXPORT virtual const CTiglUIDObject* GetNextUIDParent() const;

        TIGL_EXPORT CTiglUIDManager& GetUIDManager();
        TIGL_EXPORT const CTiglUIDManager& GetUIDManager() const;

        TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;

        TIGL_EXPORT virtual const double& GetEta() const;
        TIGL_EXPORT virtual void SetEta(const double& value);

        TIGL_EXPORT virtual const double& GetXsi() const;
        TIGL_EXPORT virtual void SetXsi(const double& value);

        TIGL_EXPORT virtual const std::string& GetReferenceUID() const;
        TIGL_EXPORT virtual void SetReferenceUID(const std::string& value);

    protected:
        void* m_parent;
        const std::type_info* m_parentType;

        CTiglUIDManager* m_uidMgr;

        /// Relative spanwise position. Eta refers to the segment or componentSegment depending on the referenced uID.
        double      m_eta;

        /// Relative chordwise position. Xsi refers to the segment or componentSegment depending on the referenced uID.
        double      m_xsi;

        /// This reference uID determines the reference coordinate system.
        /// If it points to a segment, then the eta-xsi values are considered to be in segment
        /// eta-xsi coordinates; if it points to a componentSegment,
        /// then componentSegment eta-xsi coordinates are used.
        std::string m_referenceUID;

    private:
        TIGL_EXPORT const CTiglUIDObject* GetNextUIDObject() const final;
        TIGL_EXPORT void NotifyUIDChange(const std::string& oldUid, const std::string& newUid) final;

        CPACSEtaXsiPoint(const CPACSEtaXsiPoint&) = delete;
        CPACSEtaXsiPoint& operator=(const CPACSEtaXsiPoint&) = delete;

        CPACSEtaXsiPoint(CPACSEtaXsiPoint&&) = delete;
        CPACSEtaXsiPoint& operator=(CPACSEtaXsiPoint&&) = delete;
    };
} // namespace generated

// CPACSEtaXsiPoint is customized, use type CCPACSEtaXsiPoint directly
} // namespace tigl
