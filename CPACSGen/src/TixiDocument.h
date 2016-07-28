#pragma once

#include "IOHelper.h"

namespace tigl {
	class TixiDocument {
	public:
		TixiDocument(const std::string& filename) {
			auto ret = tixiOpenDocument(filename.c_str(), &m_handle);
			if (ret != ReturnCode::SUCCESS)
				throw TixiError(ret, "Failed to open document " + filename);
		}

		~TixiDocument() {
			tixiCloseDocument(m_handle);
		}

		std::string textAttribute(const std::string& xpath, const std::string& attribute) const {
			return TixiGetTextAttribute(m_handle, xpath, attribute);
		}

		int namedChildCount(const std::string& xpath, const std::string& child) const {
			return TixiGetNamedChildrenCount(m_handle, xpath, child);
		}

		template <typename Func>
		void forEachChild(const std::string& xpath, const std::string& child, Func func) const {
			const auto count = namedChildCount(xpath.c_str(), child.c_str());
			for (int i = 1; i <= count; i++)
				func(xpath + "/" + child + "[" + std::to_string(i) + "]");
		}

		bool checkAttribute(const std::string& xpath, const std::string& attribute) const {
			return TixiCheckAttribute(m_handle, xpath, attribute);
		}

		bool checkElement(const std::string& xpath) const {
			return TixiCheckElement(m_handle, xpath);
		}

		bool checkElement(const std::string& xpath, const std::string& element) const {
			return TixiCheckElement(m_handle, xpath, element);
		}

		const TixiDocumentHandle& handle() const {
			return m_handle;
		}

	private:
		TixiDocumentHandle m_handle;
	};
}
