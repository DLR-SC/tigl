#pragma once

#include "Table.h"

// custom types in Tigl which inherit from the generated ones
class CustomTypesTable : public Table {
public:
	CustomTypesTable()
		: Table({
			// maps generated classes to actual tigl classes
			{ "CPACSFuselage",                      "CCPACSFuselage" },
			{ "CPACSFuselages",                     "CCPACSFuselages" },
			{ "CPACSProfileGeometry",               "CCPACSFuselageProfile" },
			{ "CPACSFuselageProfiles",              "CCPACSFuselageProfiles" },
			{ "CPACSFuselageSegment",               "CCPACSFuselageSegment" },
			{ "CPACSFuselageSegments",              "CCPACSFuselageSegments" },
			{ "CPACSFuselageSection",               "CCPACSFuselageSection" },
			{ "CPACSFuselageSections",              "CCPACSFuselageSections" },
			{ "CPACSFuselageElement",               "CCPACSFuselageSectionElement" },
			{ "CPACSFuselageElements",              "CCPACSFuselageSectionElements" },
			{ "CPACSPositioning",                   "CCPACSPositioning" },
			{ "CPACSPositionings",                  "CCPACSPositionings" },
			{ "CPACSTransformation",                "CCPACSTransformation" },
			{ "CPACSComponentSegment",              "CCPACSWingComponentSegment" },
			{ "CPACSComponentSegments",             "CCPACSWingComponentSegments" },
			{ "CPACSWing",                          "CCPACSWing" },
			{ "CPACSWings",                         "CCPACSWings" },
			{ "CPACSWingCell",                      "CCPACSWingCell" },
			{ "CPACSWingCells",                     "CCPACSWingCells" },
			{ "CPACSWingElement",                   "CCPACSWingSectionElement" },
			{ "CPACSWingElements",                  "CCPACSWingSectionElements" },
			{ "CPACSWingSection",                   "CCPACSWingSection" },
			{ "CPACSWingSections",                  "CCPACSWingSections" },
			{ "CPACSWingComponentSegmentStructure", "CCPACSWingCSStructure" },
			{ "CPACSWingShell",                     "CCPACSWingShell" },
			{ "CPACSWingSegment",                   "CCPACSWingSegment" },
			{ "CPACSWingSegments",                  "CCPACSWingSegments" },
			{ "CPACSCst2D",                         "CCPACSWingProfileCST" },
	}) {}
};
