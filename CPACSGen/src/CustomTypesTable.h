#pragma once

#include "Table.h"

// custom types in Tigl which inherit from the generated ones
class CustomTypesTable : public MappingTable {
public:
	CustomTypesTable()
		: MappingTable({
			// maps generated classes to actual tigl classes
			{ "CPACSSymmetryType0",                 "TiglSymmetryAxis" },

			{ "CPACSAircraftModel",                 "CCPACSModel" },
			{ "CPACSMaterialDefinition",            "CCPACSMaterial" },
			{ "CPACSFarField",                      "CCPACSFarField" },
			{ "CPACSGuideCurve",                    "CCPACSGuideCurve" },
			{ "CPACSGuideCurves",                   "CCPACSGuideCurves" },
			{ "CPACSGuideCurveProfileGeometry",     "CCPACSGuideCurveProfile" },
			{ "CPACSGuideCurveProfiles",            "CCPACSGuideCurveProfiles" },
			{ "CPACSPositioning",                   "CCPACSPositioning" },
			{ "CPACSPositionings",                  "CCPACSPositionings" },
			{ "CPACSTransformation",                "CCPACSTransformation" },
			{ "CPACSStringVectorBase",              "CCPACSStringVector" },

			{ "CPACSFuselage",                      "CCPACSFuselage" },
			{ "CPACSFuselages",                     "CCPACSFuselages" },
			{ "CPACSFuselageProfiles",              "CCPACSFuselageProfiles" },
			{ "CPACSFuselageSegment",               "CCPACSFuselageSegment" },
			{ "CPACSFuselageSegments",              "CCPACSFuselageSegments" },
			{ "CPACSFuselageSection",               "CCPACSFuselageSection" },
			{ "CPACSFuselageSections",              "CCPACSFuselageSections" },
			{ "CPACSFuselageElement",               "CCPACSFuselageSectionElement" },
			{ "CPACSFuselageElements",              "CCPACSFuselageSectionElements" },

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
