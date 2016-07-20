#pragma once

#include "Table.h"

namespace tigl {
	// contains types from the schema which should be substituted
	class TypeSubstitutionTable : public MappingTable {
	public:
		TypeSubstitutionTable()
			: MappingTable({
				{ "stringUIDBaseType", "std::string" },
				{ "stringBaseType", "std::string" },
				{ "doubleBaseType", "double" },
				{ "integerBaseType", "int" },
				{ "booleanBaseType", "bool" },
		}) {}
	};
}
