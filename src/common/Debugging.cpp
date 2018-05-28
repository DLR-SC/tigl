#include "Debugging.h"

#include <boost/filesystem.hpp>
#include <BRepTools.hxx>

#include <sstream>
#include <cctype>

#include "CTiglLogging.h"
#include "tiglcommonfunctions.h"

namespace tigl
{
    namespace
    {
        void createDirs(const std::string& path)
        {
            const auto f = boost::filesystem::path{ path };
            if (f.has_parent_path())
                boost::filesystem::create_directories(f.parent_path());
        }
    }

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
        : m_counter(0), m_outputDir(outputDir)
    {
        for (char& c : m_outputDir) {
            if (!std::isalnum(c)) {
                c = '_';
            }
        }
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

    DebugScope::~DebugScope() {
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
}
