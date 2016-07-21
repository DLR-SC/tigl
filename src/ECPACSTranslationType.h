#pragma once

namespace tigl {
	enum ECPACSTranslationType {
		ABS_LOCAL,
		ABS_GLOBAL
	};

	inline std::string ECPACSTranslationTypeToString(const ECPACSTranslationType& value) {
		switch (value) {
			case ABS_LOCAL:  return "absLocal";
			case ABS_GLOBAL: return "absGlobal";
			default: throw std::runtime_error("Invalid enum value \"" + std::to_string(static_cast<int>(value)) + "\" for enum type ECPACSTranslationType");
		}
	}

	inline ECPACSTranslationType stringToECPACSTranslationType(const std::string& value) {
		if (value == "absLocal")  return ABS_LOCAL;
		if (value == "absGlobal") return ABS_GLOBAL;
		throw std::runtime_error("Invalid string value \"" + value + "\" for enum type ECPACSTranslationType");
	}
}
