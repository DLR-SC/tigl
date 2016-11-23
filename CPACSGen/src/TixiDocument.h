#pragma once

#include "TixiHelper.h"

namespace tigl {
    namespace tixihelper {
        class TixiDocument {
        public:
            TixiDocument(const std::string& filename) {
                auto ret = tixiOpenDocument(filename.c_str(), &m_handle);
                if (ret != ReturnCode::SUCCESS)
                    throw TixiError(ret, "Failed to open document " + filename);
                RegisterNamespaces(m_handle);
            }

            ~TixiDocument() {
                tixiCloseDocument(m_handle);
            }

            bool checkAttribute(const std::string& xpath, const std::string& attribute) const {
                return TixiCheckAttribute(m_handle, xpath, attribute);
            }

            bool checkElement(const std::string& xpath) const {
                return TixiCheckElement(m_handle, xpath);
            }

            int namedChildCount(const std::string& xpath) const {
                return TixiGetNamedChildrenCount(m_handle, xpath);
            }

            template <typename Func>
            void forEachChild(const std::string& xpath, Func func) const {
                const auto count = namedChildCount(xpath);
                for (int i = 1; i <= count; i++)
                    func(xpath + "[" + std::to_string(i) + "]");
            }

            std::string textAttribute(const std::string& xpath, const std::string& attribute) const {
                return TixiGetTextAttribute(m_handle, xpath, attribute);
            }

            double doubleAttribute(const std::string& xpath, const std::string& attribute) const {
                return TixiGetDoubleAttribute(m_handle, xpath, attribute);
            }

            bool boolAttribute(const std::string& xpath, const std::string& attribute) const {
                return TixiGetBoolAttribute(m_handle, xpath, attribute);
            }

            int intAttribute(const std::string& xpath, const std::string& attribute) const {
                return TixiGetIntAttribute(m_handle, xpath, attribute);
            }

            std::string textElement(const std::string& xpath) const {
                return TixiGetTextElement(m_handle, xpath);
            }

            double doubleElement(const std::string& xpath) const {
                return TixiGetDoubleElement(m_handle, xpath);
            }

            bool boolElement(const std::string& xpath) const {
                return TixiGetBoolElement(m_handle, xpath);
            }

            int intElement(const std::string& xpath) const {
                return TixiGetIntElement(m_handle, xpath);
            }

            const TixiDocumentHandle& handle() const {
                return m_handle;
            }

        private:
            TixiDocumentHandle m_handle;
        };
    }
}