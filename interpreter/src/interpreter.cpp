#include <iostream>

#include <interpreter.hpp>
#include <extension/extension.hpp>

#include <lang/lang.hpp>

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
            
            auto it = StdClass->methods.find(std::string(function_name));

            if(it == StdClass->methods.end()) {
                it = object->cls->methods.find(std::string(function_name));

                if(it == object->cls->methods.end()) {
                    throw std::runtime_error("method not found");
                }
            }

            for(auto& param : source_code.childrens()) {
                switch (param.type())
                {
                case uva::lang::parser::ast_node_type::ast_node_valuedecl: {
                    if(param.token().type() == uva::lang::lexer::token_type::token_literal) { 
                        params_to_call.push_back(node_to_object(param));
                    }
                    else {
                        // an identifier
                        auto it = current_context.variables.find(param.token().content());

                        if(it != current_context.variables.end()) {
                            params_to_call.push_back(it->second);
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

            return call(object, it->second, params_to_call);
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_vardecl: {
            std::string_view var_name = source_code.decname();

            current_context.variables[std::string(var_name)] = node_to_object(source_code.childrens()[1]);
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_conditional: {
            std::shared_ptr<uva::lang::object> ret = execute(*source_code.condition());

            if(ret && ret->is_present()) {
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

std::shared_ptr<uva::lang::object> uva::lang::interpreter::call(std::shared_ptr<uva::lang::object> object, const uva::lang::method &method, std::vector<std::shared_ptr<uva::lang::object>> params)
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
    FalseClass  = std::make_shared<uva::lang::structure>("FalseClass");
    TrueClass   = std::make_shared<uva::lang::structure>("TrueClass");
    StringClass = std::make_shared<uva::lang::structure>("StringClass");
    IntegerClass = std::make_shared<uva::lang::structure>("IntegerClass");

    FalseClass->methods = {
        {"is_present", uva::lang::method("is_present", method_storage_type::instance_method, {}, [this](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            return std::make_shared<uva::lang::object>(FalseClass);
        })}
    };

    TrueClass->methods = {
        {"is_present", uva::lang::method("is_present", method_storage_type::instance_method, {}, [this](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            return std::make_shared<uva::lang::object>(TrueClass);
        })}
    };

    StringClass->methods = {
        {"is_present", uva::lang::method("is_present", method_storage_type::instance_method, {}, [this](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string* value = object->as<std::string>();

            if(value->empty()) {
                return std::make_shared<uva::lang::object>(FalseClass);
            }

            return std::make_shared<uva::lang::object>(TrueClass);
        })},
        {"to_s", uva::lang::method("to_s", method_storage_type::instance_method, {}, [this](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string* value = object->as<std::string>();

            std::shared_ptr<uva::lang::object> obj = std::make_shared<uva::lang::object>(StringClass);
            obj->native = new std::string(*value);

            return obj;
        })}
    };

    IntegerClass->methods = {
        {"is_present", uva::lang::method("is_present", method_storage_type::instance_method, {}, [this](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int i = *object->as<int>();
            
            if(i == 0) {
                return std::make_shared<uva::lang::object>(FalseClass);
            }

            return std::make_shared<uva::lang::object>(TrueClass);
        })},
        {"to_s", uva::lang::method("to_s", method_storage_type::instance_method, {}, [this](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int value = *object->as<int>();

            std::shared_ptr<uva::lang::object> obj = std::make_shared<uva::lang::object>(StringClass);
            obj->native = new std::string(std::to_string(value));

            return obj;
        })}
    };

    this->load(FalseClass);
    this->load(TrueClass);
    this->load(StringClass);
    this->load(IntegerClass);

    for(auto& extension : extensions) {
        extension->load_in_interpreter(this);
    }

    //TODO separate it

    // std class
    StdClass = std::make_shared<uva::lang::structure>("std");

    StdClass->methods["print"] = uva::lang::method("print", method_storage_type::instance_method, {"message"}, [](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
        std::shared_ptr<uva::lang::object> obj = params[0]->cls->methods["to_s"].call(params[0].get());
        std::cout << *obj->as<std::string>();

        return nullptr;
    });

    StdClass->methods["puts"] = uva::lang::method("puts", method_storage_type::instance_method, {"message"}, [](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
        std::shared_ptr<uva::lang::object> obj = params[0]->cls->methods["to_s"].call(params[0].get());
        std::cout << *obj->as<std::string>() << std::endl;

        return nullptr;
    });
}

const std::shared_ptr<uva::lang::object> uva::lang::interpreter::node_to_object(const uva::lang::parser::ast_node& node)
{
    if(node.token().type() == uva::lang::lexer::token_type::token_literal) {
        switch(node.token().kind())
        {
            case lexer::token_kind::token_boolean: {
                if(node.token().content() == "true") {
                    return std::make_shared<uva::lang::object>(TrueClass);
                } else {
                    return std::make_shared<uva::lang::object>(FalseClass);
                }
            }
            break;
            case lexer::token_kind::token_integer: {
                return std::make_shared<uva::lang::object>(nullptr);
            }
            case lexer::token_kind::token_string: {
                std::shared_ptr<uva::lang::object> obj = std::make_shared<uva::lang::object>(StringClass);
                obj->native = new std::string(node.token().content());
                return obj;
            }
            break;
            default:    
                throw std::runtime_error("interpreter: unknown node kind");
            break;
        }
    } else if(node.type() == uva::lang::parser::ast_node_type::ast_node_fn_call) {
        return execute(node);
    }

    auto declname = node.child_from_type(uva::lang::parser::ast_node_type::ast_node_declname);

    if(declname) {
        auto it = current_context.variables.find(declname->token().content());

        if(it != current_context.variables.end()) {
            return it->second;
        }
    }

    return nullptr;
}