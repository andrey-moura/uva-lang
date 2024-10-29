#include <iostream>

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

std::shared_ptr<uva::lang::object> uva::lang::interpreter::execute(uva::lang::parser::ast_node source_code, std::shared_ptr<uva::lang::object> object)
{
    switch (source_code.type())
    {
        case uva::lang::parser::ast_node_type::ast_node_classdecl: {
            std::string_view class_name = source_code.decname();

            auto cls = std::make_shared<uva::lang::structure>(std::string(class_name));

            for(auto& method : source_code.childrens()) {
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
        case uva::lang::parser::ast_node_type::ast_node_valuedecl: {
            return node_to_object(source_code);
        }
        break;
        case uva::lang::parser::ast_node_fn_call: {
            std::string_view function_name = source_code.decname();
            
            auto it = Std->methods.find(std::string(function_name));

            if(it == Std->methods.end()) {
                it = object->cls->methods.find(std::string(function_name));

                if(it == object->cls->methods.end()) {
                    throw std::runtime_error("method not found");
                }
            }

            std::map<std::string, std::shared_ptr<uva::lang::object>> params_to_call;

            for(auto& param : source_code.childrens()) {
                switch (param.type())
                {
                case uva::lang::parser::ast_node_type::ast_node_valuedecl: {
                    if(param.token().type() == uva::lang::lexer::token_type::token_literal) { 
                        //const std::string& value = param.token().content();
                        //params_to_call.push_back(value);
                    }
                    else {
                        // an identifier
                        auto it = current_context.variables.find(param.token().content());

                        if(it != current_context.variables.end()) {
                            //params_to_call.push_back(it->second);
                        } else {
                            throw std::runtime_error("variable not found");
                        }
                    }
                }
                    break;
                default:
                    break;
                }
            }

            call(object, it->second, {});// params_to_call);
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_vardecl: {
            std::string_view var_name = source_code.decname();
            std::string_view var_value = source_code.value();

            //current_context[std::string(var_name)] = std::string(var_value);
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_conditional: {
            std::shared_ptr<uva::lang::object> ret = execute(*source_code.condition());

            if(ret->is_present()) {
                auto if_childs = source_code.childrens();

                for(size_t i = 2; i < if_childs.size(); i++) {
                    auto statment_child = if_childs[i];
                    execute(statment_child);
                }
            }
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_context:
            execute_all(source_code, object);
        break;
        case uva::lang::parser::ast_node_type::ast_node_condition: {
            return node_to_object(source_code.childrens().front());
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_fn_return: {
            return node_to_object(source_code.childrens().front());
        }
    default:
        source_code.token().throw_error_at_current_position("interpreter: Unexpected token");
        break;
    }

    return nullptr;
}

std::shared_ptr<uva::lang::object> uva::lang::interpreter::execute_all(uva::lang::parser::ast_node source_code, std::shared_ptr<uva::lang::object> object)
{
    std::shared_ptr<uva::lang::object> result = nullptr;

    for(auto& child : source_code.childrens()) {
        result = execute(child, object);
    }

    return result;
}

std::shared_ptr<uva::lang::object> uva::lang::interpreter::instantiate(std::shared_ptr<uva::lang::structure> cls, const std::string &name)
{
    auto obj = std::make_shared<uva::lang::object>(cls);

    global_context.variables[name] = obj;

    return obj;
}

std::shared_ptr<uva::lang::object> uva::lang::interpreter::call(std::shared_ptr<uva::lang::object> object, const uva::lang::method &method, const var &params)
{
    push_context();

    if(method.block_ast.childrens().size()) {
        return execute(*method.block_ast.block(), object);
    } else if(method.function) {
        return method.function(object.get(), params);
    }

    return nullptr;
}

void uva::lang::interpreter::init()
{
    False = std::make_shared<uva::lang::structure>("FalseClass");
    True = std::make_shared<uva::lang::structure>("TrueClass");

    False->methods = {
        {"is_present", uva::lang::method("is_present", method_storage_type::instance_method, {}, [this](uva::lang::object* object, const var& params) {
            return std::make_shared<uva::lang::object>(False);
        })}
    };

    True->methods = {
        {"is_present", uva::lang::method("is_present", method_storage_type::instance_method, {}, [this](uva::lang::object* object, const var& params) {
            return std::make_shared<uva::lang::object>(True);
        })}
    };

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

const std::shared_ptr<uva::lang::object> uva::lang::interpreter::node_to_object(const uva::lang::parser::ast_node& node) const
{
    switch(node.token().kind())
    {
        case lexer::token_kind::token_boolean: {
            if(node.token().content() == "true") {
                return std::make_shared<uva::lang::object>(True);
            } else {
                return std::make_shared<uva::lang::object>(False);
            }
        }
        break;
        case lexer::token_kind::token_integer: {
            return std::make_shared<uva::lang::object>(nullptr);
        }
    }

    throw std::runtime_error("interpreter: unknown node kind");
}