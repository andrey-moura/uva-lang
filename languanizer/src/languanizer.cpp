#include <vector>
#include <string>
#include <fstream>

#include <uva/string.hpp>
#include <uva/var.hpp>
#include <console.hpp>
#include <clang-c/Index.h>  // This is libclang.

using namespace uva;

std::string to_string(CXString str)
{
    std::string s = clang_getCString(str);
    clang_disposeString(str);
    return s;
}

std::ostream& operator<<(std::ostream& stream, const CXString& str)
{
    stream << clang_getCString(str);
    clang_disposeString(str);
    return stream;
}

enum class method_storage_type {
    instance_method,
    class_method,
};

struct argument
{
    std::string name;
    std::string type;
    std::string type_name;
    std::string default_value;
    bool pointer = false;
    bool is_any = false;
    bool to_string = false;
};

class method
{
public:
    std::string name;
    std::vector<argument> args;
    method_storage_type type;
    std::string ret_type_name;

    bool operator==(const method& other) const {
        return name == other.name;
    }
};

struct _class
{
    std::string name;
    std::vector<method> methods;
};

static std::vector<_class> classes;
static bool debug = false;

CXChildVisitResult parameter_iterator(CXCursor c, CXCursor paarent, CXClientData client_data) {
    CXCursorKind kind = clang_getCursorKind(c);

    switch(kind) {
        case CXCursor_IntegerLiteral:
        case CXCursor_FloatingLiteral:
        case CXCursor_StringLiteral:
        case CXCursor_CXXBoolLiteralExpr:
        case CXCursor_UnexposedExpr:
        case CXCursor_DeclRefExpr: {

            argument* arg = (argument*)client_data;

            c = clang_getCursorReferenced(c);
            std::string default_value = to_string(clang_getCursorSpelling(c));

            arg->default_value = default_value;

            if (clang_getCursorKind(c) == CXCursor_EnumConstantDecl) {
                CXCursor enumTypeDecl = clang_getCursorSemanticParent(c);

                if (clang_getCursorKind(enumTypeDecl) == CXCursor_EnumDecl) {
                    // Obtém o tipo do enum
                    CXType enumType = clang_getCursorType(enumTypeDecl);
                    CXString enumTypeSpelling = clang_getTypeSpelling(enumType);

                    // Exibe o tipo do enum
                    std::string enum_name = to_string(enumTypeSpelling);

                    if(!enum_name.starts_with('(')) {
                        arg->default_value = enum_name + "::" + to_string(clang_getCursorSpelling(c));
                        arg->type_name = enum_name;
                    }
                }
            }

            break;
        }
    }

    return CXChildVisit_Continue;
}

CXChildVisitResult method_iterator(CXCursor c, CXCursor paarent, CXClientData client_data) {
    if(c.kind == CXCursorKind::CXCursor_ParmDecl) {
        method* m = (method*)client_data;
        argument arg;
        
        CXType type = clang_getCursorType(c);
        CXType canonical_type = clang_getCanonicalType(type);
        CXTypeKind type_kind = type.kind;

        arg.name = to_string(clang_getCursorSpelling(c));
        arg.type_name = to_string(clang_getTypeSpelling(type));

        arg.is_any = arg.name.starts_with("any_");
        if(arg.is_any) {
            arg.name.erase(0, 4);
        }

        if(arg.type_name.starts_with("const")) {
            arg.type_name.erase(0, 5);
        }

        if(arg.type_name.ends_with('&')) {
            arg.type_name.pop_back();
        }

        if(arg.type_name.ends_with("const")) {
            arg.type_name.erase(arg.type_name.size() - 5, 5);
        }

        if(arg.type_name.ends_with('*')) {
            arg.pointer = true;
            arg.type_name.pop_back();
        }

        while(arg.type_name.ends_with(' ')) {
            arg.type_name.pop_back();
        }

        while(arg.type_name.starts_with(' ')) {
            arg.type_name.erase(0, 1);
        }

        if(arg.type_name.starts_with("any_to_string")) {
            arg.to_string = true;
            arg.type_name = "std::string";
        }

        clang_visitChildren(c, &parameter_iterator, &arg);
        
        m->args.push_back(arg);
    }
    return CXChildVisit_Continue;
}

method extract_method(CXCursor class_c) {
    std::string name = to_string(clang_getCursorSpelling(class_c));
    CX_CXXAccessSpecifier access_specifier = clang_getCXXAccessSpecifier(class_c);
    if(debug) {
        std::cout << "found method declaration: " << name;
    }

    if(access_specifier != CX_CXXAccessSpecifier::CX_CXXPublic && access_specifier != CX_CXXAccessSpecifier::CX_CXXInvalidAccessSpecifier) {
        if(debug) {
            std::cout << " but it is not public, skipping..." << std::endl;
        }
        return {};
    }
    
    CX_StorageClass storage_class = clang_Cursor_getStorageClass(class_c);

    CXType return_type = clang_getCursorResultType(class_c);

    method m;
    m.name = name;
    m.type = storage_class == CX_StorageClass::CX_SC_None ? method_storage_type::instance_method : method_storage_type::class_method;
    m.ret_type_name = to_string(clang_getTypeSpelling(return_type));

    clang_visitChildren(class_c, &method_iterator, &m);

    return m;
}

_class* nmspace;

int main(int argc, char** argv) {
    if(argc < 2) {
        uva::console::log_error("stop: missing argument");
        return 1;
    }

    std::filesystem::path input = argv[1];

    if(!std::filesystem::exists(input)) {
        uva::console::log_error("input file '{}' does not exists", input.string());
        return 1;
    }

    std::map<std::string, std::string> definitions;
    std::vector<std::string> includes;

    for(size_t i = 2; i < argc; ++i) {
        std::string_view arg = argv[i];

        if(arg.starts_with('-')) {
            arg.remove_prefix(1);

            if(arg.starts_with('-')) {
                arg.remove_prefix(1);
            }

            if(arg.starts_with('D')) {
                arg.remove_prefix(1);

                if(arg.empty()) {
                    console::log_error("expected argument after '-D'");
                    return 1;
                }

                std::string name;
                std::string value;

                while(arg.size() && !arg.starts_with('=')) {
                    name.push_back(arg.front());
                    arg.remove_prefix(1);
                }

                if(arg.front() != '=') {
                    console::log_error("expected '=' after definition name");
                    return 1;
                }

                arg.remove_prefix(1);

                while(!arg.empty()) {
                    value.push_back(arg.front());
                    arg.remove_prefix(1);
                }

                if(debug) {
                    std::cout << std::format("defined '{}' as '{}'", name, value);
                }
                definitions[name] = value;
            }
    #ifndef NDEBUG
            else if(arg == "debug") {
                debug = true;
            } else if(arg == "I") {
                if(i + 1 >= argc) {
                    console::log_error("expected argument after '-I'");
                    return 1;
                }

                includes.push_back(argv[i+1]);
                i++;
            }
    #endif
            else {
                console::log_error("unknown option '{}'", arg);
                return 1;
            }
        }
    }

    CXIndex index = clang_createIndex(0, 0);

    std::vector<std::string> args = {
        
    };

    for(const auto& [name, value] : definitions) {
        args.push_back(std::format("-D{}={}", name, value).c_str());
    }

    for(const auto& include : includes) {
        args.push_back("-I" + include);
    }

    std::vector<const char*> cargs;
    for(const auto& arg : args) {
        cargs.push_back(arg.c_str());
    }

    cargs.push_back("-std=c++11");

    CXTranslationUnit unit = clang_parseTranslationUnit(
        index,
        input.string().c_str(),
        cargs.data(),
        cargs.size(),
        nullptr, 0,
        CXTranslationUnit_None
    );

    if (unit == nullptr)
    {
        console::log_error("Unable to parse translation unit. Quitting.");
        return 1;
    }

    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    
    clang_visitChildren(cursor, [](CXCursor c, CXCursor parent, CXClientData client_data) {
        CXTranslationUnit unit = *(CXTranslationUnit*)client_data;
        CXTranslationUnit cursor_unit = clang_Cursor_getTranslationUnit(c);

        if(cursor_unit != unit) {
            return CXChildVisit_Continue;
        }

        if (clang_Location_isFromMainFile (clang_getCursorLocation(c)) == 0) {
            //return CXChildVisit_Continue;
        }

        if(c.kind == CXCursorKind::CXCursor_PreprocessingDirective) {
            CXString str = clang_getCursorSpelling(c);
            std::string name = to_string(str);
            clang_disposeString(str);

            if(name.starts_with("#include")) {
                return CXChildVisit_Continue;
            }
        }

        if(c.kind == CXCursorKind::CXCursor_ClassDecl) {
            std::string name = to_string(clang_getCursorSpelling(c));
            if(clang_isCursorDefinition(c)) {
                if(debug) {
                    std::cout << "Found class '" << name << "'\n";
                }

                _class cls;
                cls.name = name;

                clang_visitChildren(c, [](CXCursor class_c, CXCursor paarent, CXClientData calient_data) {
                    //std::cout << class_c.kind << clang_getCursorKindSpelling(class_c.kind) << "\n";
                    if(class_c.kind == CXCursorKind::CXCursor_CXXMethod) {
                        if(clang_isCursorDefinition(class_c)) {
                            _class* c = (_class*)calient_data;

                            method m = extract_method(class_c);
                            if(m.name.size()) {
                                c->methods.push_back(m);
                            }
                        }
                    }

                    return CXChildVisit_Continue;
                }, &cls);

                classes.push_back(cls);
            }
            
            return CXChildVisit_Continue;
        } else if(c.kind == CXCursorKind::CXCursor_FunctionDecl) {
            // std::string name = to_string(clang_getCursorSpelling(c));
            // if(debug) {
            //     std::cout << "Found function '" << name << "'\n";
            // }

            // method m = extract_method(c);
            // if(m.name.size()) {
            //     classes.front().methods.push_back(m);
            // }
        }

        return CXChildVisit_Continue;
    }, &unit);

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);

    std::filesystem::path output_file_name = input.parent_path();
    output_file_name /= "src";
    output_file_name /= input.stem();
    output_file_name.replace_extension(".cpp");

    std::ofstream ofile(output_file_name);

    time_t t;
    struct tm *tmp;
    const char* fmt = "%a, %d %b %y %T %z";
    char outstr[200];

    t = time(NULL);
    tmp = gmtime(&t);

    strftime(outstr, sizeof(outstr), fmt, tmp);

    ofile << "//This file was generated with uva-languanizer. Modifications to this file may be overridden." << std::endl;
    ofile << "//This file has no licence, simply use it as you need." << std::endl;
    ofile << "//Generated at: " << outstr << "." << std::endl;
    ofile << std::endl;

    ofile << "#include <interpreter/interpreter.hpp>" << std::endl;
    ofile << "#include <lang/object.hpp>" << std::endl;
    ofile << "#include <lang/method.hpp>" << std::endl;
    ofile << "#include <lang/class.hpp>" << std::endl;

    ofile << "#include <lang/" << input.filename().string() << ">" << std::endl;
    ofile << std::endl;

    for(auto& cls : classes) {
        ofile << "std::shared_ptr<uva::lang::structure> create_" << cls.name << "(uva::lang::interpreter* interpreter)" << std::endl;
        ofile << "{" << std::endl;

        std::string class_name_without_class = cls.name;

        size_t class_pos = class_name_without_class.find("_class");
        
        if(class_pos != std::string::npos) {
            class_name_without_class.erase(class_pos, 6);
        }

        ofile << "\treturn std::make_shared<uva::lang::structure>(uva::lang::structure(\"" << class_name_without_class << "\"";

        ofile << ", {" << std::endl << std::endl;;

        for(size_t i = 0; i < cls.methods.size(); ++i) {
            const auto& method = cls.methods[i];
            bool is_overloaded = false;

            for(const auto& method : cls.methods) {
                if(&method != &cls.methods[i] && method.name == cls.methods[i].name) {
                    is_overloaded = true;

                    ofile << std::endl << std::endl;
                    ofile << "\t/*" << std::endl;
                    ofile << "\t//This method is duplicated. Method overloading is not allowed in uvalang." << std::endl;
                    ofile << "\t//It needs to be manually implemented." << std::endl;

                    uva::console::log_warning("method '{}' is duplicated in class '{}'", cls.methods[i].name, cls.name);
                    break;
                }
            }

            std::string prettier_name = uva::string::to_snake_case(method.name);

            if(method.ret_type_name == "void" && prettier_name.starts_with("is_")) {
                prettier_name.erase(0, 3);
                prettier_name.push_back('?');
            }

            ofile << "\t\tuva::lang::method(\"" << prettier_name << "\", ";

            ofile << "uva::lang::method_storage_type::";

            if(method.type == method_storage_type::class_method) {
                ofile << "class_method, ";
            } else {
                ofile << "instance_method, ";
            }
            
            ofile << "{ ";

            for(size_t i = 0; i < method.args.size(); ++i) {
                if(i) {
                    ofile << ", ";
                }

                ofile << "\"" << method.args[i].name << "\"";
            }

            ofile << " }, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {" << std::endl;

            for(size_t argument_i = 0; argument_i < method.args.size(); ++argument_i) {
                const auto& argument = method.args[argument_i];

                if(argument.to_string) {
                    ofile << "\t\t\t" << argument.type_name;

                    std::string argument_type_name_snake_case = uva::string::to_snake_case(argument.type_name);

                    ofile << " " << argument.name << " = " << "params[" << argument_i << "]";

                    ofile << "->to_var().to_s();" << std::endl;
                }
                else {
                    ofile << "\t\t\tconst " << argument.type_name << "& " << argument.name << " = " << "params[" << argument_i << "]->as<" << argument.type_name << ">();" << std::endl;
                }
            }

            if(method.args.size()) {
                ofile << std::endl;
            }

            if(method.type == method_storage_type::instance_method) {
                ofile << "\t\t\t" << cls.name << "& native_object = object->as<" << cls.name << ">();" << std::endl;
            }

            bool has_been_moved = false;

            if(method.ret_type_name == "void") {
                ofile << "\t\t\t";
            }
            else {
                if(method.ret_type_name.ends_with("_instance")) {
                    std::string ret_class = method.ret_type_name;
                    ret_class.erase(ret_class.size() - 9, 9);
                    
                    //if(ret_class == "object") {
                        ofile << "\t\t\treturn uva::lang::object::instantiate(interpreter, interpreter->";

                        ret_class[0] = std::toupper(ret_class[0]);
                        ofile << ret_class << "Class";
                        ofile << ", std::move(";
                        has_been_moved = true;
                    //} else {
                    //}
                } else {
                    ofile << "\t\t\treturn ";
                }
            }

            if(method.type == method_storage_type::class_method) {
                ofile << cls.name << "::" << method.name << "(";
            } else {
                ofile << "native_object." << method.name << "(";
            }

            for(size_t argument_i = 0; argument_i < method.args.size(); ++argument_i) {
                const auto& argument = method.args[argument_i];

                if(argument_i) {
                    ofile << ", ";
                }

                ofile << argument.name;
            }

            ofile << ")";

            if(method.ret_type_name == "void") {
                ofile << ";" << std::endl << "\t\t\treturn nullptr;" << std::endl;
            } else {
                if(has_been_moved) {
                    ofile << "));" << std::endl;
                } else {
                    ofile << ";" << std::endl;
                }
            }

            ofile << "\t\t})";

            if(is_overloaded) {
                ofile << std::endl << "\t,*/";
            } else {
                if(i + 1 < cls.methods.size()) {
                    ofile << "," << std::endl;
                }
            }

            ofile << std::endl;
        }

        ofile << std::endl << "\t}));";
        ofile << std::endl << "}" << std::endl;
    }

    return 0;
}