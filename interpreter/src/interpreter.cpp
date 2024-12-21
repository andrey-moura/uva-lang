#include <iostream>

#include <uva/file.hpp>

#include <interpreter.hpp>
#include <extension/extension.hpp>

#include <lang/lang.hpp>

//extern std::vector<uva::lang::extension*> extensions;

uva::lang::interpreter::interpreter()
{
    init();
}

void uva::lang::interpreter::load(std::shared_ptr<uva::lang::structure> cls)
{
    cls->methods["call"] = uva::lang::method("call", uva::lang::method_storage_type::instance_method, {"fn, params"}, [this, cls](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
        const std::string& method_name = params[0]->as<std::string>();

        auto method_it = cls->methods.find(method_name);

        if(method_it == cls->methods.end()) {
            throw std::runtime_error("class " + cls->name + " does not have a function called " + method_name);
        }

        std::vector<std::shared_ptr<uva::lang::object>> method_params = params;
        if(method_params.size() > 1) {
            method_params.erase(method_params.begin());
        }

        return call(cls, object, method_it->second, method_params);
    });

    classes.push_back(cls);
}

std::shared_ptr<uva::lang::object> uva::lang::interpreter::execute(uva::lang::parser::ast_node source_code, std::shared_ptr<uva::lang::object>& object)
{
    switch (source_code.type())
    {
        case uva::lang::parser::ast_node_type::ast_node_fn_decl: {
            
            std::string_view method_name = source_code.decname();

            std::vector<std::string> params;
            params.reserve(source_code.childrens().size());

            for(auto& param : source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_fn_params)->childrens()) {
                params.push_back(param.token().content());
            }

            current_context.functions[std::string(method_name)] = uva::lang::method(std::string(method_name), method_storage_type::instance_method, params, source_code);
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_classdecl: {
            std::string_view class_name = source_code.decname();

            auto cls = std::make_shared<uva::lang::structure>(std::string(class_name));

            auto baseclass_node = source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_classdecl_base);

            if (baseclass_node)
            {
                std::string base_class_name;
                auto decname_node = baseclass_node->child_from_type(uva::lang::parser::ast_node_type::ast_node_declname);

                if(decname_node->childrens().size() == 0) {
                    base_class_name = baseclass_node->decname();
                } else {
                    auto object_node = decname_node->child_from_type(uva::lang::parser::ast_node_type::ast_node_fn_object);

                    const uva::lang::parser::ast_node& object_node_child = object_node->childrens().front();

                    base_class_name = object_node_child.token().content() + "." + decname_node->token().content();
                }

                auto base_class = find_class(base_class_name);

                if(!base_class) {
                    throw std::runtime_error("base class " + std::string(base_class_name) + " not found");
                }

                cls->base = base_class;
                base_class->deriveds.push_back(cls);
            }

            for(auto& class_child : source_code.context()->childrens()) {
                switch (class_child.type())
                {
                case uva::lang::parser::ast_node_type::ast_node_fn_decl: {
                    std::string_view method_name = class_child.decname();

                    std::vector<std::string> params;
                    params.reserve(class_child.childrens().size());

                    for(auto& param : class_child.child_from_type(uva::lang::parser::ast_node_type::ast_node_fn_params)->childrens()) {
                        params.push_back(param.token().content());
                    }

                    cls->methods[std::string(method_name)] = uva::lang::method(std::string(method_name), method_storage_type::instance_method, params, class_child);
                }
                break;
                case uva::lang::parser::ast_node_type::ast_node_vardecl: {
                    std::string_view var_name = class_child.decname();
                    cls->instance_variables[std::string(var_name)] = NullClass;
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

            // This is a pointer to a shared pointer because the object can be changed
            std::shared_ptr<uva::lang::object>* object_to_call_ptr = nullptr;
            // And we have a shared_ptr in case the object is created, os it still alive in the current context
            std::shared_ptr<uva::lang::object> object_to_call = nullptr;

            std::shared_ptr<uva::lang::structure> class_to_call = nullptr;

            if(auto it = current_context.functions.find(source_code.decname()); it != current_context.functions.end()) {
                method_to_call = &it->second;
            }

            uva::lang::parser::ast_node* object_node = source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_fn_object);

            const std::string& function_name(source_code.decname());
            bool is_super = function_name == "super";
            bool is_assignment = function_name == "=";

            if(!method_to_call && object_node) {
                // function call from a class/object/function return value

                object_node = object_node->childrens().data();

                if(object_node->type() == uva::lang::parser::ast_node_type::ast_node_declname) {
                    const std::string& class_or_object_name = object_node->token().content();

                    std::map<std::string, std::shared_ptr<uva::lang::object>>::iterator object_it;

                    if(object) {
                        object_it = object->instance_variables.find(class_or_object_name);

                        if(object_it != object->instance_variables.end()) {
                            object_to_call_ptr = &object_it->second;
                        }
                    }

                    if(!object_to_call_ptr) {
                        object_it = current_context.variables.find(class_or_object_name);

                        if(object_it != current_context.variables.end()) {
                            object_to_call_ptr = &object_it->second;
                        }
                    }

                    if(object_to_call_ptr) {
                        object_to_call = *object_to_call_ptr;
                    }

                    if(object_to_call) {
                        if(is_assignment) {
                            const auto& params_node = source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_fn_params);

                            object_to_call = node_to_object(params_node->childrens().front());
                            *object_to_call_ptr = object_to_call;

                            return object;
                        } else {
                            auto method_it = object_to_call->cls->methods.find(function_name);
                            
                            if(method_it == object_to_call->cls->methods.end()) {
                                if(object_to_call->cls->base) {
                                    method_it = object_to_call->cls->base->methods.find(function_name);

                                    if(method_it != object_to_call->cls->base->methods.end()) {
                                        method_to_call = &method_it->second;
                                        class_to_call = object_to_call->cls->base;
                                        object_to_call = object_to_call->base_instance;
                                    } else {
                                        throw std::runtime_error("class " + object_to_call->cls->name + " does not have a function called " + function_name);
                                    }
                                } else {
                                    throw std::runtime_error("class " + object_to_call->cls->name + " does not have a function called " + function_name);
                                }
                            } else {
                                method_to_call = &method_it->second;
                                class_to_call = object_to_call->cls;
                            }
                        }
                    } else {
                        for(auto& cls : classes) {
                            if(cls->name == class_or_object_name) {
                                auto it = cls->methods.find(function_name);

                                if(it == cls->methods.end()) {
                                    throw std::runtime_error("class " + class_or_object_name + " does not have a function called " + function_name);
                                }

                                method_to_call = &it->second;
                                class_to_call = cls;
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
                        throw std::runtime_error(object_node->token().error_message_at_current_position("undefined operator '.' for null"));
                    }

                    auto it = object_to_call->cls->methods.find(std::string(function_name));

                    if(it == object_to_call->cls->methods.end()) {
                        throw std::runtime_error("class " + object_to_call->cls->name + " does not have a method called " + std::string(function_name));
                    }

                    method_to_call = &it->second;
                    class_to_call = object_to_call->cls;
                } else if(object_node->type() == uva::lang::parser::ast_node_type::ast_node_valuedecl) {
                    for(auto& cls : classes) {
                        if(cls->name == object_node->token().content()) {
                            class_to_call = cls;
                            break;
                        }
                    }

                    if(class_to_call) {
                        auto it = class_to_call->methods.find(std::string(function_name));

                        if(it == class_to_call->methods.end()) {
                            throw std::runtime_error("class " + class_to_call->name + " does not have a method called " + std::string(function_name));
                        }

                        method_to_call = &it->second;
                    } else {
                        std::shared_ptr<uva::lang::structure> object_class = nullptr;

                        if(object) {
                            object_class = object->cls;
                        }

                        object_to_call = node_to_object(*object_node, object_class, object);

                        auto it = object_to_call->cls->methods.find(std::string(function_name));

                        if(it == object_to_call->cls->methods.end()) {
                            throw std::runtime_error("class " + object_to_call->cls->name + " does not have a method called " + std::string(function_name));
                        }

                        method_to_call = &it->second;
                        class_to_call = object_to_call->cls;
                    }
                }
            } else {
                if(is_super) {
                    if(!object) {
                        throw std::runtime_error("super can only be called from an instance object");
                    }

                    if(!object->cls->base) {
                        throw std::runtime_error("class " + object->cls->name + " does not have a base class");
                    }

                    auto it = object->cls->base->methods.find("new");

                    if(it == object->cls->base->methods.end()) {
                        throw std::runtime_error("base class " + object->cls->base->name + " does not have a constructor");
                    }

                    method_to_call = &it->second;
                    object_to_call = object;
                    class_to_call = object->cls->base;
                } else {
                    if(object) {
                        auto it = object->cls->methods.find(function_name);

                        if(it == object->cls->methods.end()) {
                            if(object->cls->base) {
                                it = object->cls->base->methods.find(function_name);

                                if(it != object->cls->base->methods.end()) {
                                    if(!object->base_instance) {
                                        throw std::runtime_error("object has no base instance");
                                    }

                                    method_to_call = &it->second;
                                    class_to_call = object->cls->base;
                                    object_to_call = object->base_instance;
                                } else {
                                    // ?????
                                    // Why was it throwing exceptions?
                                    //throw std::runtime_error("function '" + function_name + "' not found in class " + object->cls->name);
                                }
                            }
                        } else {
                            method_to_call = &it->second;
                            object_to_call = object;
                            class_to_call = object->cls;
                        }
                    } 
                    
                    if(!method_to_call) {
                        auto it = StdClass->methods.find(function_name);

                        if(it == StdClass->methods.end()) {
                            throw std::runtime_error("function '" + function_name + "' not found");
                        } else {
                            method_to_call = &it->second;
                            class_to_call = StdClass;
                        }
                    }
                }
            }

            std::vector<std::shared_ptr<uva::lang::object>> params_to_call;

            uva::lang::parser::ast_node* params_node = source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_fn_params);

            if(params_node) {
                for(auto& param : params_node->childrens()) {
                    switch (param.type())
                    {
                    case uva::lang::parser::ast_node_type::ast_node_arraydecl:
                    case uva::lang::parser::ast_node_type::ast_node_dictionarydecl:
                    case uva::lang::parser::ast_node_type::ast_node_valuedecl: {
                        if(param.type() == uva::lang::parser::ast_node_type::ast_node_arraydecl || uva::lang::parser::ast_node_type::ast_node_dictionarydecl || param.token().type() == uva::lang::lexer::token_type::token_literal) { 
                            params_to_call.push_back(node_to_object(param));
                        }
                        else {
                            throw std::runtime_error("interpreter: Unexpected token in function call parameters");
                        }
                    }
                        break;
                    case uva::lang::parser::ast_node_type::ast_node_declname: {
                        // an identifier
                        auto it = current_context.variables.find(param.token().content());

                        if(it != current_context.variables.end()) {
                            params_to_call.push_back(it->second);
                        } else {
                            throw std::runtime_error("'" + std::string(param.token().content()) + "' is undefined");
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

            std::shared_ptr<uva::lang::object> ret = call(class_to_call, object_to_call, *method_to_call, params_to_call);

            if(is_super) {
                object_to_call->base_instance = ret;
                return nullptr;
            }

            return ret;
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_vardecl: {
            std::string_view var_name = source_code.decname();
            std::shared_ptr<uva::lang::structure> cls = nullptr;
            if(object) {
                cls = object->cls;
            }
            current_context.variables[std::string(var_name)] = node_to_object(source_code.childrens()[1], cls, object);
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_conditional: {
            std::shared_ptr<uva::lang::object> ret = execute(*source_code.condition(), object);

            if(ret && ret->is_present()) {
                auto context = source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_context);

                ret = execute(*context, object);
            } else {
                auto e = source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_else);
                
                if(e) {
                    auto else_context = e->child_from_type(uva::lang::parser::ast_node_type::ast_node_context);

                    ret = execute(*else_context, object);
                }
            }

            return ret;
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_while: {
            std::shared_ptr<uva::lang::object> ret = nullptr;

            while(execute(*source_code.condition(), object)->is_present()) {
                ret = execute(*source_code.context(), object);

                if(current_context.has_returned) {
                    return current_context.return_value;
                }
            }

            return ret;
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_break: {
            current_context.has_returned = true;
            return nullptr;
        }
        case uva::lang::parser::ast_node_type::ast_node_context:
            return execute_all(source_code, object);
        break;
        case uva::lang::parser::ast_node_type::ast_node_condition: {
            return node_to_object(source_code.childrens().front());
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_fn_return: {
            if(source_code.childrens().size()) {
                return node_to_object(source_code.childrens().front());
            } else {
                return std::make_shared<uva::lang::object>(NullClass);
            }
        }
        break;
        case uva::lang::parser::ast_node_type::ast_node_foreach: {
            auto* valuedecl = source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_valuedecl);

            std::shared_ptr<uva::lang::object> array_or_dictionary = node_to_object(*valuedecl);

            auto* vardecl = source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_vardecl);

            std::string var_name(vardecl->decname());

            if(array_or_dictionary->cls == ArrayClass) {
                std::vector<std::shared_ptr<uva::lang::object>>& array_values = array_or_dictionary->as<std::vector<std::shared_ptr<uva::lang::object>>>();
                for(auto& value : array_values) {
                    current_context.variables[var_name] = value;
                    execute_all(*source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_context), object);
                }
            } else if(array_or_dictionary->cls == DictionaryClass) {
                uva::lang::dictionary& dictionary_values = array_or_dictionary->as<uva::lang::dictionary>();
                for(auto& [key, value] : dictionary_values) {
                    std::vector<std::shared_ptr<uva::lang::object>> params = { key, value };
                    std::shared_ptr<uva::lang::object> params_object = uva::lang::object::instantiate(this, ArrayClass, params);

                    current_context.variables[var_name] = params_object;

                    execute_all(*source_code.child_from_type(uva::lang::parser::ast_node_type::ast_node_context), object);
                }
            } else {
                throw std::runtime_error("foreach should iterate over an array or a dictionary");
            }
        }
        break;
    default:
        throw std::runtime_error(source_code.token().error_message_at_current_position("interpreter: Unexpected token"));
        break;
    }

    return nullptr;
}

std::shared_ptr<uva::lang::object> uva::lang::interpreter::execute_all(std::vector<uva::lang::parser::ast_node>::const_iterator begin, std::vector<uva::lang::parser::ast_node>::const_iterator end, std::shared_ptr<uva::lang::object>& object)
{
    std::shared_ptr<uva::lang::object> result = nullptr;

    for(auto it = begin; it != end; it++) {
        const uva::lang::parser::ast_node& node = *it;

        if(node.type() == uva::lang::parser::ast_node_type::ast_node_undefined && node.token().type() == uva::lang::lexer::token_type::token_eof) {
            break;
        }

        result = execute(*it, object);

        if(it->type() == uva::lang::parser::ast_node_type::ast_node_fn_return) {
            current_context.has_returned = true;
            current_context.return_value = result;
            return result;
        } else if(current_context.has_returned) {
            return current_context.return_value;
        }
    }

    return nullptr;
}

std::shared_ptr<uva::lang::object> uva::lang::interpreter::execute_all(uva::lang::parser::ast_node source_code, std::shared_ptr<uva::lang::object>& object)
{
    return execute_all(source_code.childrens().begin(), source_code.childrens().end(), object);
}

std::shared_ptr<uva::lang::object> uva::lang::interpreter::call(std::shared_ptr<uva::lang::structure> cls, std::shared_ptr<uva::lang::object> object, const uva::lang::method &method, std::vector<std::shared_ptr<uva::lang::object>> params)
{
    push_context();

    bool is_constructor = method.name == "new";

    if(is_constructor) {
        // Special case
        // The object is created before the method is called
        // If the object was instantiated in from native code, it will be passed as a parameter
        if(!object) {
            object = std::make_shared<uva::lang::object>(cls);
        }
    }

    std::shared_ptr<uva::lang::object> ret = nullptr;

    if(method.block_ast.childrens().size()) {
        for(size_t i = 0; i < method.params.size(); i++) {
            current_context.variables[method.params[i]] = params[i];
        }
        
        ret = execute(*method.block_ast.block(), object);
    } else if(method.function) {
        ret = method.function(object, params);
    }

    if(is_constructor) {
        if(ret) {
            throw std::runtime_error("constructor should not return a value");
        }

        ret = object;
    }

    pop_context();

    return ret;
}

void uva::lang::interpreter::init()
{
    this->load(FalseClass   = uva::lang::false_class::create());
    this->load(TrueClass    = uva::lang::true_class::create(this));
    this->load(StringClass  = uva::lang::string_class::create(this));
    this->load(IntegerClass = uva::lang::integer_class::create(this));
    this->load(FileClass    = uva::lang::file_class::create(this));
    this->load(StdClass     = uva::lang::std_class::create(this));
    this->load(ArrayClass   = uva::lang::array_class::create(this));
    this->load(DictionaryClass = uva::lang::dictionary_class::create(this));
    this->load(NullClass    = uva::lang::null_class::create(this));
}

const std::shared_ptr<uva::lang::object> uva::lang::interpreter::node_to_object(const uva::lang::parser::ast_node& node, std::shared_ptr<uva::lang::structure> cls, std::shared_ptr<uva::lang::object> object)
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
                std::shared_ptr<uva::lang::object> obj = uva::lang::object::instantiate(this, IntegerClass, std::stoi(node.token().content()));
                return obj;
            }
            break;
            case lexer::token_kind::token_string: {
                std::shared_ptr<uva::lang::object> obj = uva::lang::object::instantiate(this, StringClass, std::move(std::string(node.token().content())));
                return obj;
            }
            break;
            case lexer::token_kind::token_null:
                return std::make_shared<uva::lang::object>(NullClass);
            break;
            default:    
                throw std::runtime_error("interpreter: unknown node kind");
            break;
        }
    } else if(node.type() == uva::lang::parser::ast_node_type::ast_node_fn_call) {
        return execute(node, object);
    } else if(node.type() == uva::lang::parser::ast_node_type::ast_node_declname || node.type() == uva::lang::parser::ast_node_type::ast_node_valuedecl) {
        if(object) {
            auto it = object->instance_variables.find(node.token().content());

            if(it != object->instance_variables.end()) {
                return it->second;
            }
        }

        auto it = current_context.variables.find(node.token().content());

        if(it != current_context.variables.end()) {
            return it->second;
        }

        throw std::runtime_error("'" + std::string(node.token().content()) + "' is undefined");
    } else if(node.type() == uva::lang::parser::ast_node_type::ast_node_arraydecl) {
        std::vector<std::shared_ptr<uva::lang::object>> array;

        for(auto& child : node.childrens()) {
            array.push_back(node_to_object(child));
        }

        return uva::lang::object::instantiate(this, ArrayClass, std::move(array));
    } else if(node.type() == uva::lang::parser::ast_node_type::ast_node_dictionarydecl) {
        uva::lang::dictionary map;

        for(auto& child : node.childrens()) {
            const uva::lang::parser::ast_node* name_node = child.child_from_type(uva::lang::parser::ast_node_type::ast_node_declname);
            const uva::lang::parser::ast_node* value_node = child.child_from_type(uva::lang::parser::ast_node_type::ast_node_valuedecl);

            std::shared_ptr<uva::lang::object> key   = node_to_object(name_node->childrens().front());
            std::shared_ptr<uva::lang::object> value = node_to_object(value_node->childrens().front());

            map.push_back({ key, value });
        }

        return uva::lang::object::instantiate(this, DictionaryClass, std::move(map));
    }

    throw std::runtime_error("interpreter: unknown node type");

    return nullptr;
}

void uva::lang::interpreter::load_extension(uva::lang::extension* extension)
{
    extension->load(this);
    extensions.push_back(extension);
}

void uva::lang::interpreter::start_extensions()
{
    for(auto& extension : extensions) {
        extension->start(this);
    }
}
