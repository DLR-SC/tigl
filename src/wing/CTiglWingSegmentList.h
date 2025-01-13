/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2017-05-16 Martin Siggel <martin.siggel@dlr.de>
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
#pragma once

#include <CSharedPtr.h>

#include <vector>

namespace tigl
{

class CCPACSWingSegment;

class CTiglWingSegmentList
{
public:
    CTiglWingSegmentList(const CTiglWingSegmentList& ex)
        : pimpl(ex.pimpl)
    {
    }

    template< typename T >
    CTiglWingSegmentList(T const & x ) : pimpl( new Model<T>(x) )
    {
    }

    friend std::vector<CCPACSWingSegment*> getSortedSegments(CTiglWingSegmentList const& list)
    {
        return list.pimpl->getSegments_();
    }

private:
    struct Concept
    {
        virtual ~Concept() {}
        virtual std::vector<CCPACSWingSegment*> getSegments_() const = 0;
    };

    template< typename T >
    struct Model : Concept
    {
        Model(T const & x) : object(x) { }
        std::vector<CCPACSWingSegment*> getSegments_() const override {return getSortedSegments(object);}

        T const & object;
    };

    CSharedPtr<Concept> pimpl;
};

} // namespace tigl
