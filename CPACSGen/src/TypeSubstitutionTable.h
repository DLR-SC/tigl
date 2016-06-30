#pragma once

#include "Table.h"

// contains types from the schema which should be substituted
class TypeSubstitutionTable : public Table {
public:
	TypeSubstitutionTable()
		: Table({
			{ "stringUIDBaseType", "std::string" },
			{ "stringBaseType", "std::string" },
			{ "doubleBaseType", "double" },
			{ "integerBaseType", "int" },
			{ "booleanBaseType", "bool" },
		}) {}
};
