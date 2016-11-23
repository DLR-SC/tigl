#pragma once

#include "TixiHelper.h"

namespace tigl {
    namespace tixihelper {
        class TixiDocument {
            static const TixiDocumentHandle invalidTixiHandle = -1;
        public:
            TixiDocument()
                : m_handle(invalidTixiHandle) {}

            explicit TixiDocument(TixiDocumentHandle handle)
                : m_handle(handle) {}
            
            TixiDocument(const TixiDocument&) = delete;
            TixiDocument& operator=(const TixiDocument&) = delete;

            TixiDocument(TixiDocument&& other) {
                swap(other);
            }

            TixiDocument& operator=(TixiDocument&& other) {
                swap(other);
                return *this;
            }

            ~TixiDocument() {
                if (m_handle != invalidTixiHandle)
                    tixiCloseDocument(m_handle);
            }

            static TixiDocument createFromFile(const std::string& filename) {
                TixiDocument doc(TixiOpenDocument(filename));
                doc.registerNamespaces();
                return doc;
            }

            static TixiDocument createNew(const std::string& rootElement) {
                return TixiDocument(TixiCreateDocument(rootElement));
            }

            static TixiDocument createFromString(const std::string& xml) {
                TixiDocument doc(TixiImportFromString(xml));
                doc.registerNamespaces();
                return doc;
            }

            void addCpacsHeader(const std::string& name, const std::string& creator, const std::string& version, const std::string& description, const std::string& cpacsVersion) {
                TixiAddCpacsHeader(m_handle, name, creator, version, description, cpacsVersion);
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

            std::vector<std::string> attributeNames(const std::string& xpath) const {
                return TixiGetAttributeNames(m_handle, xpath);
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

            std::time_t timeTElement(const std::string& xpath) const {
                return TixiGetTimeTElement(m_handle, xpath);
            }

            void saveAttribute(const std::string& xpath, const std::string& attribute, const char* value) {
                return TixiSaveAttribute(m_handle, xpath, attribute, value);
            }

            void saveAttribute(const std::string& xpath, const std::string& attribute, const std::string& value) {
                return TixiSaveAttribute(m_handle, xpath, attribute, value);
            }

            void saveAttribute(const std::string& xpath, const std::string& attribute, double value) {
                return TixiSaveAttribute(m_handle, xpath, attribute, value);
            }

            void saveAttribute(const std::string& xpath, const std::string& attribute, bool value) {
                return TixiSaveAttribute(m_handle, xpath, attribute, value);
            }

            void saveAttribute(const std::string& xpath, const std::string& attribute, int value) {
                return TixiSaveAttribute(m_handle, xpath, attribute, value);
            }

            void saveElement(const std::string& xpath, const char* value) {
                return TixiSaveElement(m_handle, xpath, value);
            }

            void saveElement(const std::string& xpath, const std::string& value) {
                return TixiSaveElement(m_handle, xpath, value);
            }

            void saveElement(const std::string& xpath, double value, const std::string& format = "%g") {
                return TixiSaveElement(m_handle, xpath, value, format);
            }

            void saveElement(const std::string& xpath, bool value) {
                return TixiSaveElement(m_handle, xpath, value);
            }

            void saveElement(const std::string& xpath, int value) {
                return TixiSaveElement(m_handle, xpath, value);
            }

            void saveElement(const std::string& xpath, std::time_t value) {
                return TixiSaveElement(m_handle, xpath, value);
            }

            void createElement(const std::string& xpath) {
                TixiCreateElement(m_handle, xpath);
            }

            // includes creation of all parent elements
            void createElements(const std::string& xpath) {
                TixiCreateElements(m_handle, xpath);
            }

            void removeAttribute(const std::string& xpath, const std::string& attribute) {
                TixiRemoveAttribute(m_handle, xpath, attribute);
            }

            void removeElement(const std::string& xpath) {
                TixiRemoveElement(m_handle, xpath);
            }

            std::string toString() const {
                return TixiExportDocumentAsString(m_handle);
            }

            void registerNamespaces() {
                TixiRegisterNamespacesFromDocument(m_handle);
            }

            const TixiDocumentHandle& handle() const {
                return m_handle;
            }

            TixiDocumentHandle releaseHandle() {
                auto h =  m_handle;
                m_handle = invalidTixiHandle;
                return h;
            }

        private:
            void swap(TixiDocument& other) {
                using std::swap;
                swap(m_handle, other.m_handle);
            }

            TixiDocumentHandle m_handle = invalidTixiHandle;
        };
    }
}