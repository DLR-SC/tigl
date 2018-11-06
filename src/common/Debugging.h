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

#pragma once

#include <boost/atomic.hpp>
#include <string>
#include <map>

class TopoDS_Shape;

namespace tigl
{
void dumpShape(const TopoDS_Shape& shape, const std::string& outputDir, const std::string& filename, int counter = -1);

// represents a point in the execution of a program at which shapes can be dumped
class TracePoint
{
public:
    TracePoint(const std::string& outputDir);

    void operator++(int);
    int hitCount() const;
    void dumpShape(const TopoDS_Shape& shape, const std::string& filename);

private:
    std::string m_outputDir;
    boost::atomic<int> m_counter;
};

// creates a trace point at the location of this macro with the specified variable name and output directory
// the trace point is static and incremented each time programm flow executes over the location of this macro
#define TRACE_POINT_OUTPUT_DIR(variableName, outputDir) static ::tigl::TracePoint variableName(outputDir); variableName++

// uses the function inside which the macro is expanded as output directory, __FUNCTION__ may not be supported by each compiler
#define TRACE_POINT(variableName) TRACE_POINT_OUTPUT_DIR(variableName, std::string("CrashInfo/") + __FUNCTION__)

// allows dumping shapes in case an object of this class is destroyed by an exception
class DebugScope
{
public:
    DebugScope(TracePoint& tp);
    ~DebugScope();

    // stores shape and dumps it in case the scope is left by exception
    void addShape(const TopoDS_Shape& shape, const std::string& filename);

    // dumps the shape to the filesystem immediately
    void dumpShape(const TopoDS_Shape& shape, const std::string& filename);

private:
    TracePoint& m_tp;
    std::map<std::string, TopoDS_Shape> m_shapes;
};

// creates a debug scope object at the location of this macro with the specified variable name
// furthermore, a tracepoint is created, used by the debug scope to dump shapes in case of leaving the current scope by an exception
#define DEBUG_SCOPE(variableName) TRACE_POINT(variableName##TracePoint); ::tigl::DebugScope variableName(variableName##TracePoint)
}
