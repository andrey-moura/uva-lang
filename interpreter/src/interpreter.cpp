#include <interpreter.hpp>

#include <extension/extension.hpp>

extern std::vector<uva::lang::extension*> extensions;

uva::lang::interpreter::interpreter()
{
    init();
}

void uva::lang::interpreter::load(std::shared_ptr<uva::lang::structure> cls)
{
    classes.push_back(cls);
}

void uva::lang::interpreter::execute(uva::lang::parser::ast_node source_code)
{
    for(auto& child : source_code.childrens()) {
        switch (child.type())
        {
            case uva::lang::parser::ast_node_type::ast_node_classdecl: {
                std::string_view class_name = child.decname();

                auto cls = std::make_shared<uva::lang::structure>(std::string(class_name));

                for(auto& method : child.childrens()) {
                    switch (method.type())
                    {
                    case uva::lang::parser::ast_node_type::ast_node_fn_decl: {
                        std::string_view method_name = method.decname();

                        cls->methods[std::string(method_name)] = uva::lang::method(std::string(method_name), method_storage_type::instance_method, {}, method);
                    }
                        break;
                    default:
                        break;
                    }
                }

                load(cls);
            }
            break;
        default:
            child.token().throw_error_at_current_position("interpreter: Unexpected token");
            break;
        }
    }
}

std::shared_ptr<uva::lang::object> uva::lang::interpreter::instantiate(std::shared_ptr<uva::lang::structure> cls, const std::string &name)
{
    auto obj = std::make_shared<uva::lang::object>(cls);

    global_context.variables[name] = obj;

    return obj;
}

std::shared_ptr<uva::lang::object> uva::lang::interpreter::call(std::shared_ptr<uva::lang::object> object, const uva::lang::method &method, const var &params)
{
    std::map<std::string, var> variables;

    if(method.block_ast.childrens().size()) {
        for(auto& statment : method.block_ast.childrens()) {
            switch (statment.type())
            {
            case uva::lang::parser::ast_node_fn_call: {
                    std::string_view function_name = statment.decname();
                    
                    auto it = Std->methods.find(std::string(function_name));

                    if(it == Std->methods.end()) {
                        it = object->cls->methods.find(std::string(function_name));

                        if(it == object->cls->methods.end()) {
                            throw std::runtime_error("method not found");
                        }
                    }

                    var params_to_call = var::array();

                    for(auto& param : statment.childrens()) {
                        switch (param.type())
                        {
                        case uva::lang::parser::ast_node_type::ast_node_valuedecl: {
                                const std::string& value = param.token().content();
                                params_to_call.push_back(value);
                            }
                            break;
                        default:
                            break;
                        }
                    }

                    call(object, it->second, params_to_call);
                }
                break;
            // case uva::lang::lexer::cursor_type::cursor_return: {
            //     std::string_view return_value = statment.value();

            //     if(return_value == "true") {
            //         return std::make_shared<uva::lang::object>(True);
            //     } else if(return_value == "false") {
            //         return std::make_shared<uva::lang::object>(False);
            //     }
            // }
            //     break;
            // case uva::lang::lexer::cursor_type::cursor_var: {
            //     std::string_view var_name = statment.decname();
            //     std::string_view var_value = statment.value();
            //     std::string param;

            //     if(var_value == "true") {
            //         //variables[std::string(var_name)] = True;
            //     } else if(var_value == "false") {
            //         //variables[std::string(var_name)] = False;
            //     } else {
            //         if(var_value.starts_with("\"") && var_value.ends_with("\"")) {
            //             var_value.remove_prefix(1);
            //             var_value.remove_suffix(1);

            //             param.reserve(var_value.size());

            //             bool escape = false;

            //             for(const char& c : var_value) {
            //                 switch (c)
            //                 {
            //                 case '\\':
            //                     escape = true;
            //                     break;
            //                 case 'n': {
            //                     if(escape) {
            //                         param.push_back('\n');
            //                     } else {
            //                         param.push_back(c);
            //                     }
            //                 }
            //                 break;
            //                 default:
            //                     escape = false;
            //                     param.push_back(c);
            //                     break;
            //                 }
            //             }
            //         }

            //         variables[std::string(var_name)] = std::string(param);
            //     }
            // }
                break;
            default:
                break;
            }
        }
    } else if(method.function) {
        return method.function(object.get(), params);
    }

    return nullptr;
}

void uva::lang::interpreter::init()
{
    False = std::make_shared<uva::lang::structure>("FalseClass");
    True = std::make_shared<uva::lang::structure>("TrueClass");

    this->load(False);
    this->load(True);

    for(auto& extension : extensions) {
        extension->load_in_interpreter(this);
    }

    //TODO separate it

    // std class
    Std = std::make_shared<uva::lang::structure>("std");

    Std->methods["print"] = uva::lang::method("print", method_storage_type::instance_method, {"message"}, [](uva::lang::object* object, const var& params) {
        std::cout << params[0].to_s();
        return nullptr;
    });

    Std->methods["puts"] = uva::lang::method("puts", method_storage_type::instance_method, {"message"}, [](uva::lang::object* object, const var& params) {
        std::cout << params[0].to_s() << std::endl;
        return nullptr;
    });
}