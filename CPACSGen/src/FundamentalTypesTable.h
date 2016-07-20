#pragma once

#include "Table.h"

namespace tigl {
	// maps fundamental types to there Tixi IO functions
	class FundamentalTypesTable : public MappingTable {
	public:
		FundamentalTypesTable()
			: MappingTable({
				{ "std::string", "Text" },
				{ "double", "Double" },
				{ "bool", "Bool" },
				{ "int", "Int" },
				{ "time_t", "TimeT" }
		}) {}
	};
}