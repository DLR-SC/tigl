/*
* Copyright (c) 2018 RISC Software GmbH
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

#include "Debugging.h"

#include <boost/filesystem.hpp>
#include <BRepTools.hxx>

#include <sstream>
#include <cctype>

#include "CTiglLogging.h"
#include "tiglcommonfunctions.h"

namespace
{
void createDirs(const std::string& path)
{
    boost::filesystem::path f(path);
    if (f.has_parent_path()) {
        boost::filesystem::create_directories(f.parent_path());
    }
}
} // namespace

namespace tigl
{

void dumpShape(const TopoDS_Shape& shape, const std::string& outputDir, const std::string& filename, int counter)
{
    std::stringstream ss;
    ss << outputDir << "/";
    if (counter >= 0) {
        ss << counter << '_';
    }
    ss << filename;
    ss << ".brep";
    const std::string& file = ss.str();

    try {
        createDirs(file);
        BRepTools::Write(shape, file.c_str());
    }
    catch (const std::exception& e) {
        LOG(WARNING) << "Failed to dump debug shape " << file << ": " << e.what();
    }
}

TracePoint::TracePoint(const std::string& outputDir)
    : m_outputDir(outputDir)
    , m_counter(0)
{
    for (std::string::iterator it = m_outputDir.begin(); it != m_outputDir.end(); ++it) {
        char& c = *it;
        if (!std::isalnum(c) && c != '/') {
            c = '_';
        }
    }
}

int TracePoint::hitCount() const
{
    return m_counter;
}

void TracePoint::operator++(int)
{
    m_counter++;
}

void TracePoint::dumpShape(const TopoDS_Shape& shape, const std::string& filename)
{
    tigl::dumpShape(shape, m_outputDir, filename, m_counter);
}

DebugScope::DebugScope(TracePoint& tp)
    : m_tp(tp)
{
}

DebugScope::~DebugScope()
{
    // dump shapes in case we are stack unwinding
    if (std::uncaught_exception()) {
        for (std::map<std::string, TopoDS_Shape>::iterator it = m_shapes.begin(); it != m_shapes.end(); ++it) {
            m_tp.dumpShape(it->second, it->first);
        }
    }
}

void DebugScope::addShape(const TopoDS_Shape& shape, const std::string& filename)
{
    m_shapes[filename] = shape;
}

void DebugScope::dumpShape(const TopoDS_Shape& shape, const std::string& filename)
{
    m_tp.dumpShape(shape, filename);
}

} // namespace tigl
