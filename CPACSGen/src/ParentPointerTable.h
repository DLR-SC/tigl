#pragma once

#include "Table.h"

// specifies classes for which to generate ctors accepting the pointer to their parent class
class ParentPointerTable : public Table {
public:
	ParentPointerTable()
		: Table({
			"CPACSWing",
			"CPACSWings",
			"CPACSWingCells",
			"CPACSWingSegment",
			"CPACSWingSegments",
			"CPACSComponentSegment",
			"CPACSComponentSegments",
			"CPACSFuselage",
			"CPACSFuselages",
			"CPACSFuselageSegment",
			"CPACSFuselageSegments",
			"CPACSRotorBlades"
		}) {}
};
