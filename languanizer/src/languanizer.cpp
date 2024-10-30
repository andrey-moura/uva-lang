#include <vector>
#include <string>
#include <fstream>

#include <uva/string.hpp>
#include <uva/var.hpp>
#include <console.hpp>
#include <str_stream/str_fstream.hpp>
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

enum class return_type {
    return_void,
    return_bool,
    return_other,
};

struct argument
{
    std::string name;
    std::string type;
    std::string type_name;
    std::string default_value;
    var::var_type var_type;
    bool pointer = false;
};

class method
{
public:
    std::string name;
    std::vector<argument> args;
    method_storage_type type;
    return_type ret;

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

        // Verifica se o tipo canônico é um tipo primitivo
        switch (canonical_type.kind) {
            case CXType_Void:
                arg.var_type = var::var_type::null_type;
            break;
            case CXType_Bool:
                arg.var_type = var::var_type::integer;
            break;
            case CXType_Char_U:
                arg.var_type = var::var_type::integer;
            break;
            case CXType_UChar:
                arg.var_type = var::var_type::integer;
            break;
            case CXType_Char_S:
                arg.var_type = var::var_type::integer;
            break;
            case CXType_SChar:
                arg.var_type = var::var_type::integer;
            break;
            case CXType_UShort:
                arg.var_type = var::var_type::integer;
            break;
            case CXType_UInt:
                arg.var_type = var::var_type::integer;
            break;
            case CXType_ULong:
                arg.var_type = var::var_type::integer;
            break;
            case CXType_ULongLong:
                arg.var_type = var::var_type::integer;
            break;
            case CXType_Short:
                arg.var_type = var::var_type::integer;
            break;
            case CXType_Int:
                arg.var_type = var::var_type::integer;
            break;
            case CXType_Long:
                arg.var_type = var::var_type::integer;
            break;
            case CXType_LongLong:
                arg.var_type = var::var_type::integer;
            break;
            case CXType_Float:
                arg.var_type = var::var_type::real;
            break;
            case CXType_Double:
                arg.var_type = var::var_type::real;
            break;
            case CXType_LongDouble:
                arg.var_type = var::var_type::real;
            break;
            default:
                arg.var_type = var::var_type::undefined;
            break;
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
    
    std::cout << "\n";

    CX_StorageClass storage_class = clang_Cursor_getStorageClass(class_c);

    CXType return_type = clang_getCursorResultType(class_c);

    method m;
    m.name = name;
    m.type = storage_class == CX_StorageClass::CX_SC_None ? method_storage_type::instance_method : method_storage_type::class_method;
    switch(return_type.kind) {
        case CXTypeKind::CXType_Void:
            m.ret = return_type::return_void;
            break;
        case CXTypeKind::CXType_Bool:
            m.ret = return_type::return_bool;
            break;
        default:
            m.ret = return_type::return_other;
            break;
    }

    clang_visitChildren(class_c, &method_iterator, &m);

    return m;
}

_class* nmspace;

int main(int argc, char** argv) {
    if(argc < 2) {
        //uva::console::log_error("stop: missing argument");
      //  return 1;
    }

    //std::filesystem::path input = argv[1];
    std::filesystem::path input = "/home/andrey/Downloads/wxWidgets-3.2.5/include/wx/msgdlg.h";

    if(!std::filesystem::exists(input)) {
        uva::console::log_error("input file '{}' does not exists", input.string());
        return 1;
    }

    std::map<std::string, std::string> definitions;

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
            }
    #endif
            else {
                console::log_error("unknown option '{}'", arg);
                return 1;
            }
        }
    }

    CXIndex index = clang_createIndex(0, 0);

    std::vector<const char*> args = {
        "-I/home/andrey/Downloads/wxWidgets-3.2.5/include",
        "-DwxUSE_MSGDLG=1",
        "-std=c++11",
    };

    for(const auto& [name, value] : definitions) {
        args.push_back(std::format("-D{}={}", name, value).c_str());
    }

    classes.push_back({"wx"});

    nmspace = &classes.back();
    
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index,
        input.string().c_str(),
        args.data(),
        args.size(),
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
            return CXChildVisit_Continue;
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
            std::string name = to_string(clang_getCursorSpelling(c));
            if(debug) {
                std::cout << "Found function '" << name << "'\n";
            }

            method m = extract_method(c);
            if(m.name.size()) {
                classes.front().methods.push_back(m);
            }
        }

        return CXChildVisit_Continue;
    }, &unit);

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);

    std::ofstream ofile(std::filesystem::absolute("output.hpp"));

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

    for(auto& cls : classes) {
        ofile << "std::shared_ptr<Class> " << cls.name << "Class = Class::create(\"" << cls.name << "\"";

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

            if(method.ret == return_type::return_bool && prettier_name.starts_with("is_")) {
                prettier_name.erase(0, 3);
                prettier_name.push_back('?');
            }

            ofile << "\tMethod(\"" << prettier_name << "\", ";

            if(method.type == method_storage_type::class_method) {
                ofile << "method_storage_type::class_method, ";
            } else {
                ofile << "method_storage_type::instance_method, ";
            }
            
            ofile << "{ ";

            for(size_t i = 0; i < method.args.size(); ++i) {
                if(i) {
                    ofile << ", ";
                }

                ofile << "\"" << method.args[i].name << "\"";
            }

            ofile << " }, [](Object* object, const var& params) {" << std::endl;

            for(size_t argument_i = 0; argument_i < method.args.size(); ++argument_i) {
                const auto& argument = method.args[argument_i];

                ofile << "\t\t" << argument.type_name;
                if(argument.pointer) {
                    ofile << "*";
                }

                ofile << " " << argument.name;

                if(argument.pointer) {
                    ofile << " = nullptr;";
                } else if(argument.default_value.size()) {
                    ofile << " = " << argument.default_value << ";";
                } else {
                    std::string argument_type_name_snake_case = uva::string::to_snake_case(argument.type_name);
                    if(argument.var_type == var::var_type::undefined) {
                        ofile << " = " << argument_type_name_snake_case << "_from_var(params[" << '"' << argument.name << '"' << "]);";
                    } else {
                        switch(argument.var_type) {
                            case var::var_type::integer:
                                ofile << " = params[" << '"' << argument.name << '"' << "].as<var::integer>();";
                                break;
                            case var::var_type::real:
                                ofile << " = params[" << '"' << argument.name << '"' << "].as<var::real>();";
                                break;
                            case var::var_type::string:
                                ofile << " = params[" << '"' << argument.name << '"' << "].as<var::string>();";
                                break;
                            default:
                                ofile << ";";
                                break;
                        }
                    }
                }

                ofile << std::endl;
            }

            if(method.args.size()) {
                ofile << std::endl;
            }

            if(method.type == method_storage_type::instance_method) {
                ofile << "\t\t" << cls.name << "* native_object = (" << cls.name << "*)(object);" << std::endl;
            }

            //if(method.ret == return_type::return_void) {
                ofile << "\t\t";
            //} else {
                //ofile << "\t\treturn ";
            //}

            if(method.type == method_storage_type::class_method) {
                ofile << cls.name << "::" << method.name << "(";
            } else {
                ofile << "native_object->" << method.name << "(";
            }

            for(size_t argument_i = 0; argument_i < method.args.size(); ++argument_i) {
                const auto& argument = method.args[argument_i];

                if(argument_i) {
                    ofile << ", ";
                }

                ofile << argument.name;
            }

            ofile << ");" << std::endl;

            //if(method.ret == return_type::return_void) {
                ofile << "\t\treturn nullptr;" << std::endl;
            //}

            ofile << "\t})";

            if(is_overloaded) {
                ofile << std::endl << "\t,*/";
            } else {
                if(i + 1 < cls.methods.size()) {
                    ofile << "," << std::endl;
                }
            }
        }

        ofile << std::endl << std::endl << "});" << std::endl << std::endl;
    }

    return 0;
}