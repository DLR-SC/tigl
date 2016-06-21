#pragma once

#include <string>
#include <unordered_map>

class TypeSubstitutionTable {
public:
	TypeSubstitutionTable() {
		m_table = {
			// custom types (from Airbus TiGLClassGenerator)
			{ "stringUIDBaseType", "std::string" },
			{ "stringBaseType", "std::string" },
			{ "doubleBaseType", "double" },
			{ "integerBaseType", "int" },
			{ "booleanBaseType", "bool" },
			//{ "pointXYZType", "CTiglPoint" },
			//{ "pointType", "CTiglPoint" }
		};
	}

	bool hasSubstitution(const std::string& type) const {
		return m_table.find(type) != std::end(m_table);
	}

	std::string substitute(const std::string& type) const {
		const auto it = m_table.find(type);
		if (it != std::end(m_table))
			return it->second;
		else
			return type;
	}

private:
	std::unordered_map<std::string, std::string> m_table;
};
