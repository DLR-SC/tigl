#pragma once

#include "Table.h"

class ParentPointerTable : public Table {
public:
	ParentPointerTable()
		: Table({
			{ "CPACSWingSegment", "" }
		}) {}
};