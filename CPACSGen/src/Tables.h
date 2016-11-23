#pragma once

#include "Table.h"

namespace tigl {
    class Tables {
    public:
        Tables(const std::string& inputDirectory) :
            m_customTypes      (inputDirectory + "/CustomTypes.txt"     ),
            m_fundamentalTypes (inputDirectory + "/FundamentalTypes.txt"),
            m_typeSubstitutions(inputDirectory + "/TypeSubstitution.txt"),
            m_parentPointers   (inputDirectory + "/ParentPointer.txt"   ),
            m_reservedNames    (inputDirectory + "/ReservedNames.txt"   ) {}

        MappingTable m_customTypes;
        MappingTable m_fundamentalTypes;
        MappingTable m_typeSubstitutions;

        Table m_parentPointers;
        Table m_reservedNames;
    };
}
