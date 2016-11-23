#pragma once

#include <string>

namespace tigl {
    struct Field;
    struct Class;
    struct Enum;
    class TypeSystem;
    class Tables;
    class IndentingStreamWrapper;

    class CodeGen {
    public:
        CodeGen(const std::string& outputLocation, TypeSystem& types, Tables& tables);
    private:
        struct Includes {
            std::vector<std::string> hppIncludes;
            std::vector<std::string> hppForwards;
            std::vector<std::string> hppCustomForwards;
            std::vector<std::string> cppIncludes;
        };

        TypeSystem& m_types;
        Tables&     m_tables;

        auto getterSetterType(const Field& field) const -> std::string;
        auto fieldType(const Field& field) const -> std::string;
        auto parentPointerThis(const Class& c) const -> std::string;

        void writeFields(IndentingStreamWrapper& hpp, const std::vector<Field>& fields);
        void writeAccessorDeclarations(IndentingStreamWrapper& hpp, const std::vector<Field>& fields);
        void writeAccessorImplementations(IndentingStreamWrapper& cpp, const std::string& className, const std::vector<Field>& fields);
        void writeParentPointerGetters(IndentingStreamWrapper& hpp, const Class& c);
        void writeParentPointerGetterImplementation(IndentingStreamWrapper& cpp, const Class& c);
        void writeIODeclarations(IndentingStreamWrapper& hpp, const std::string& className, const std::vector<Field>& fields);
        void writeReadAttributeOrElementImplementation(IndentingStreamWrapper& cpp, const Class& c, const Field& f);
        void writeWriteAttributeOrElementImplementation(IndentingStreamWrapper& cpp, const Field& f);
        void writeReadBaseImplementation(IndentingStreamWrapper& cpp, const std::string& type);
        void writeWriteBaseImplementation(IndentingStreamWrapper& cpp, const std::string& type);
        void writeReadImplementation(IndentingStreamWrapper& cpp, const Class& className, const std::vector<Field>& fields);
        void writeWriteImplementation(IndentingStreamWrapper& cpp, const Class& className, const std::vector<Field>& fields);
        void writeLicenseHeader(IndentingStreamWrapper& f);
        auto resolveIncludes(const Class& c) -> Includes;
        void writeCtors(IndentingStreamWrapper& hpp, const Class& c);
        void writeParentPointerFields(IndentingStreamWrapper& hpp, const Class& c);
        void writeCtorImplementations(IndentingStreamWrapper& cpp, const Class& c);
        void writeHeader(IndentingStreamWrapper& hpp, const Class& c, const Includes& includes);
        void writeSource(IndentingStreamWrapper& cpp, const Class& c, const Includes& includes);
        void writeClass(IndentingStreamWrapper& hpp, IndentingStreamWrapper& cpp, const Class& c);
        void writeEnum(IndentingStreamWrapper& hpp, const Enum& e);
    };
}
