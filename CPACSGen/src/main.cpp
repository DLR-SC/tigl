#include <iostream>

#include "SchemaParser.h"
#include "TypeSystem.h"
#include "CodeGen.h"
#include "Tables.h"

namespace tigl {
    void run(const std::string& inputDirectory, const std::string& outputDirectory) {
        // load tables
        Tables tables(inputDirectory);

        // read types and elements
        const auto& cpacsLocation = inputDirectory + "/cpacs_schema.xsd";
        std::cout << "Parsing " << cpacsLocation << std::endl;
        SchemaParser schema(cpacsLocation);

        // generate type system from schema
        TypeSystem typeSystem(schema, tables);

        // build dependencies
        typeSystem.buildDependencies();

        // apply a few reductions and cleanups
        typeSystem.collapseEnums();

        typeSystem.runPruneList();

        // generate code
        std::cout << "Generating classes" << std::endl;
        CodeGen codegen(outputDirectory, typeSystem, tables);
    }
}

int main(int argc, char* argv[]) {
    // parse command line arguments
    if (argc != 3) {
        std::cerr << "Usage: CPACSGen inputDirectory outputDirectory" << std::endl;
        return -1;
    }

    const std::string inputDirectory  = argv[1];
    const std::string outputDirectory = argv[2];

    try {
        tigl::run(inputDirectory, outputDirectory);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        return -1;
    }
}
