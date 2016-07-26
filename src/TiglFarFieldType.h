#pragma once

#include <string>
#include <stdexcept>
#include <cctype>

namespace tigl {
	enum TiglFarFieldType {
		NONE,
		HALF_SPHERE,
		FULL_SPHERE,
		HALF_CUBE,
		FULL_CUBE
	};

	inline std::string TiglFarFieldTypeToString(const TiglFarFieldType& value) {
		switch (value) {
			case TiglFarFieldType::HALF_SPHERE: return "halfSphere";
			case TiglFarFieldType::FULL_SPHERE: return "fullSphere";
			case TiglFarFieldType::HALF_CUBE:   return "halfCube";
			case TiglFarFieldType::FULL_CUBE:   return "fullCube";
			case TiglFarFieldType::NONE:        return "none";
			default: throw std::runtime_error("Invalid enum value \"" + std::to_string(static_cast<int>(value)) + "\" for enum type TiglFarFieldType");
		}
	}
	inline TiglFarFieldType stringToTiglFarFieldType(const std::string& value) {
		auto toLower = [](std::string str) { for (char& c : str) c = std::tolower(c); return str; };
		if (toLower(value) == "halfsphere") return TiglFarFieldType::HALF_SPHERE;
		if (toLower(value) == "fullsphere") return TiglFarFieldType::FULL_SPHERE;
		if (toLower(value) == "halfcube")   return TiglFarFieldType::HALF_CUBE;
		if (toLower(value) == "fullcube")   return TiglFarFieldType::FULL_CUBE;
		if (toLower(value) == "none")       return TiglFarFieldType::NONE;
		throw std::runtime_error("Invalid string value \"" + value + "\" for enum type TiglFarFieldType");
	}
}
