#include <iostream>

#include <uva/file.hpp>

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
            uva::lang::method* method_to_call = nullptr;
            std::shared_ptr<uva::lang::object> object_to_call = nullptr;

            uva::lang::parser::ast_node* object_node = source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_fn_object);

            if(object_node) {
                // function call from a class/object/function return value

                object_node = object_node->childrens().data();

                if(object_node->type() == uva::lang::parser::ast_node_type::ast_node_declname) {
                    const std::string& class_or_object_name = object_node->token().content();

                    auto object_it = current_context.variables.find(class_or_object_name);

                    if(object_it != current_context.variables.end()) {
                        object_to_call = object_it->second;

                        auto method_it = object_to_call->cls->methods.find(source_code.decname());
                        
                        if(method_it == object_to_call->cls->methods.end()) {
                            throw std::runtime_error("class " + object_to_call->cls->name + " does not have a method called " + class_or_object_name);
                        }

                        method_to_call = &method_it->second;
                    } else {
                        for(auto& cls : classes) {
                            if(cls->name == class_or_object_name) {
                                const std::string& function_name = source_code.decname();

                                auto it = cls->methods.find(function_name);

                                if(it == cls->methods.end()) {
                                    throw std::runtime_error("class " + function_name + " does not have a method called " + function_name);
                                }

                                method_to_call = &it->second;
                                break;
                            }
                        }
                    }

                    if(!method_to_call) {
                        throw std::runtime_error("class or object " + class_or_object_name + " not found");
                    }
                } else if (object_node->type() == uva::lang::parser::ast_node_type::ast_node_fn_call) {
                    object_to_call = execute(*object_node, object);

                    if(!object_to_call) {
                        object_node->token().throw_error_at_current_position("undefined operator '.' for null");
                    }

                    std::string_view function_name = source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_declname)->token().content();

                    auto it = object_to_call->cls->methods.find(std::string(function_name));

                    if(it == object_to_call->cls->methods.end()) {
                        throw std::runtime_error("class " + object_to_call->cls->name + " does not have a method called " + std::string(function_name));
                    }

                    method_to_call = &it->second;
                }
            } else {
                std::string function_name(source_code.decname());

                auto it = StdClass->methods.find(function_name);

                if(it == StdClass->methods.end()) {
                    if(object) {
                        it = object->cls->methods.find(function_name);

                        if(it == object->cls->methods.end()) {
                            throw std::runtime_error("function '" + function_name + "' not found in class " + object->cls->name);
                        }
                    } else {
                        throw std::runtime_error("function '" + function_name + "' not found");
                    }

                    method_to_call = &it->second;
                    object_to_call = object;
                } else {
                    method_to_call = &it->second;
                }
            }

            std::vector<std::shared_ptr<uva::lang::object>> params_to_call;

            uva::lang::parser::ast_node* params_node = source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_fn_params);

            if(params_node) {
                for(auto& param : params_node->childrens()) {
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
                                throw std::runtime_error("'" + std::string(param.token().content()) + "' is undefined");
                            }
                        }
                    }
                        break;
                    case uva::lang::parser::ast_node_type::ast_node_fn_call:
                        params_to_call.push_back(execute(param, object));
                    break;
                    default:
                        throw std::runtime_error("interpreter: Unexpected token in function call parameters");
                    break;
                    }
                }
            }

            return call(object_to_call, *method_to_call, params_to_call);
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

                ret = execute_all(if_childs.begin()+2, if_childs.end(), object);
            }

            return ret;
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_context:
            return execute_all(source_code, object);
        break;
        case uva::lang::parser::ast_node_type::ast_node_condition: {
            return node_to_object(source_code.childrens().front());
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_fn_return: {
            return node_to_object(source_code.childrens().front());
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_foreach: {
            auto* valuedecl = source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_valuedecl);

            std::shared_ptr<uva::lang::object> array = node_to_object(*valuedecl);

            if(array->cls != ArrayClass) {
                throw std::runtime_error("foreach should iterate over an array");
            }

            std::vector<std::shared_ptr<uva::lang::object>>& array_values = array->as<std::vector<std::shared_ptr<uva::lang::object>>>();

            auto* vardecl = source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_vardecl);

            std::string var_name(vardecl->decname());


            for(auto& value : array_values) {
                current_context.variables[var_name] = value;
                execute_all(*source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_context), object);
            }
        }
        break;
    default:
        source_code.token().throw_error_at_current_position("interpreter: Unexpected token");
        break;
    }

    return nullptr;
}

std::shared_ptr<uva::lang::object> uva::lang::interpreter::execute_all(std::vector<uva::lang::parser::ast_node>::const_iterator begin, std::vector<uva::lang::parser::ast_node>::const_iterator end, std::shared_ptr<uva::lang::object> object)
{
    std::shared_ptr<uva::lang::object> result = nullptr;

    for(auto it = begin; it != end; it++) {
        result = execute(*it, object);

        if(it->type() == uva::lang::parser::ast_node_type::ast_node_fn_return) {
            current_context.has_returned = true;
            current_context.return_value = result;
            return result;
        } else if(current_context.has_returned) {
            return current_context.return_value;
        }
    }

    return result;
}

std::shared_ptr<uva::lang::object> uva::lang::interpreter::execute_all(uva::lang::parser::ast_node source_code, std::shared_ptr<uva::lang::object> object)
{
    return execute_all(source_code.childrens().begin(), source_code.childrens().end(), object);
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
    this->load(FalseClass   = uva::lang::false_class::create());
    this->load(TrueClass    = uva::lang::true_class::create());
    this->load(StringClass  = uva::lang::string_class::create(this));
    this->load(IntegerClass = uva::lang::integer_class::create(this));
    this->load(FileClass    = uva::lang::file_class::create(this));
    this->load(StdClass     = uva::lang::std_class::create(this));
    this->load(ArrayClass   = uva::lang::array_class::create(this));

    for(auto& extension : extensions) {
        extension->load_in_interpreter(this);
    }
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
                std::shared_ptr<uva::lang::object> obj = std::make_shared<uva::lang::object>(IntegerClass);
                return instantiate(IntegerClass, std::stoi(node.token().content()));
            }
            break;
            case lexer::token_kind::token_string: {
                std::shared_ptr<uva::lang::object> obj = std::make_shared<uva::lang::object>(StringClass);
                return instantiate(StringClass, std::move(std::string(node.token().content())));
            }
            break;
            default:    
                throw std::runtime_error("interpreter: unknown node kind");
            break;
        }
    } else if(node.type() == uva::lang::parser::ast_node_type::ast_node_fn_call) {
        return execute(node);
    } else if(node.type() == uva::lang::parser::ast_node_type::ast_node_valuedecl) {
        auto it = current_context.variables.find(node.token().content());

        if(it != current_context.variables.end()) {
            return it->second;
        }
    } else if(node.type() == uva::lang::parser::ast_node_type::ast_node_arraydecl) {
        std::vector<std::shared_ptr<uva::lang::object>> array;

        for(auto& child : node.childrens()) {
            array.push_back(node_to_object(child));
        }

        return instantiate(ArrayClass, std::move(array));
    }

    return nullptr;
}