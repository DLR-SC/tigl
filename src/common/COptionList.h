/* 
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-03-29 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef COPTIONLIST_H
#define COPTIONLIST_H

#include "tigl_internal.h"
#include "CTiglError.h"
#include "typename.h"
#include "any.h"

#include <map>
#include <vector>
#include <boost/core/typeinfo.hpp>

namespace tigl
{

class COptionList
{
public:
    COptionList()
    {
    }

    template <class T>
    T Get(const std::string& name) const
    {
        OptionsMap::const_iterator it = m_options.find(name);
        if (it == m_options.end()) {
            throw CTiglError("No such option: " + name);
        }
        else {
            try {
                return tigl::any_cast<T>(it->second);
            }
            catch(const CTiglError&) {
                throw CTiglError("Cannot convert option \"" + it->first +
                                 "\" to " + typeName(typeid(T)) + ". Expecting " + 
                                 boost::core::demangled_name(it->second.type()));
            }
        }
    }

    double GetDouble(const std::string& name) const
    {
        return Get<double>(name);
    }

    int GetInt(const std::string& name) const
    {
        return Get<int>(name);
    }

    std::string GetString(const std::string& name) const
    {
        return Get<std::string>(name);
    }

    template <class T>
    void Set(const std::string& name, const T& value)
    {
        OptionsMap::iterator it = m_options.find(name);
        if (it == m_options.end()) {
            throw CTiglError("No such option: " + name);
        }
        else if (it->second.type() != typeid(T)) { // check type
            throw CTiglError("Wrong type in COptionList::SetOption. Expecting " + 
                             boost::core::demangled_name(it->second.type()));
        }
        else {
            it->second = value;
        }
    }

    void SetFromString(const std::string& name, const std::string& value)
    {
        OptionsMap::iterator it = m_options.find(name);
        if (it == m_options.end()) {
            throw CTiglError("No such option: " + name, TIGL_NOT_FOUND);
        }
        else {
            tigl::from_string(value, it->second);
        }
    }

    void SetDouble(const std::string& name, double value)
    {
        Set(name, value);
    }

    void SetInt(const std::string& name, int value)
    {
        Set(name, value);
    }

    void SetString(const std::string& name, const std::string& value)
    {
        Set(name, value);
    }

    size_t GetNOptions() const
    {
        return m_options.size();
    }
    
    std::string GetOptionName(size_t idx)
    {
        return m_iters[idx]->first;
    }

    std::string GetOptionType(size_t idx)
    {
        return boost::core::demangled_name(m_iters[idx]->second.type());
    }

    bool HasOption(const std::string& name) const
    {
        return m_options.find(name) != m_options.end();
    }

    COptionList Merged(const tigl::COptionList& other) const
    {
        COptionList l1(*this);

        for (OptionsMap::const_iterator it = other.m_options.begin();
             it != other.m_options.end(); ++it) {
            const std::string& name = it->first;
            const any& value = it->second;
            l1.AddOption(name, value);
        }

        return l1;
    }

protected:
    template <class T>
    void AddOption(const std::string& name, const T& default_value)
    {
        AddOption(name, tigl::any(default_value));
    }
    
    void AddOption(const std::string& name, const any& default_value)
    {
        size_t old_size = m_options.size();
        m_options[name] = default_value;
        if (old_size < m_options.size()) {
            m_iters.clear();
            for (OptionsMap::iterator it = m_options.begin(); it != m_options.end(); ++it) {
                m_iters.push_back(it);
            }
        }
        assert(m_iters.size() == m_options.size());
    }

private:
    typedef std::map<std::string, tigl::any> OptionsMap;
    OptionsMap m_options;
    std::vector<OptionsMap::iterator> m_iters;
};

} // namespace tigl

#endif // COPTIONLIST_H
