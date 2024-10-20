#include <vm.hpp>

#include <extension/extension.hpp>

extern std::vector<uva::lang::extension*> extensions;

uva::lang::vm::vm()
{
    init();
}

void uva::lang::vm::load(std::shared_ptr<uva::lang::structure> cls)
{
    classes.push_back(cls);
}

std::shared_ptr<uva::lang::object> uva::lang::vm::instantiate(std::shared_ptr<uva::lang::structure> cls, const std::string &name)
{
    auto obj = std::make_shared<uva::lang::object>(cls);

    global_context.variables[name] = obj;

    return obj;
}

std::shared_ptr<uva::lang::object> uva::lang::vm::call(std::shared_ptr<uva::lang::object> object, const uva::lang::method &method, const var &params)
{
    if(method.block.size()) {
        for(auto& statment : method.block_cursor.children()) {
            switch (statment.type())
            {
            case uva::lang::lexer::cursor_type::cursor_fncall: {
                    std::string_view function_name = statment.decname();
                    
                    auto it = Std->methods.find(std::string(function_name));

                    if(it == Std->methods.end()) {
                        it = object->cls->methods.find(std::string(function_name));

                        if(it == object->cls->methods.end()) {
                            throw std::runtime_error("method not found");
                        }
                    }

                    var params_to_call = var::array();

                    const uva::lang::lexer::cursor* params_cursor = statment.fncallparams();

                    for(auto& param : params_cursor->children()) {
                        std::string_view content = param.content();
                        if(content.starts_with("\"") && content.ends_with("\"")) {
                            content.remove_prefix(1);
                            content.remove_suffix(1);

                            std::string param;
                            param.reserve(content.size());

                            bool escape = false;

                            for(const char& c : content) {
                                switch (c)
                                {
                                case '\\':
                                    escape = true;
                                    break;
                                case 'n': {
                                    if(escape) {
                                        param.push_back('\n');
                                    } else {
                                        param.push_back(c);
                                    }
                                }
                                break;
                                default:
                                    escape = false;
                                    param.push_back(c);
                                    break;
                                }
                            }
                            
                            params_to_call.push_back(param);
                        }
                    }

                    call(object, it->second, params_to_call);
                }
                break;
            case uva::lang::lexer::cursor_type::cursor_return: {
                std::string_view return_value = statment.value();

                if(return_value == "true") {
                    return std::make_shared<uva::lang::object>(True);
                } else if(return_value == "false") {
                    return std::make_shared<uva::lang::object>(False);
                }
            }
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

void uva::lang::vm::init()
{
    False = std::make_shared<uva::lang::structure>("FalseClass");
    True = std::make_shared<uva::lang::structure>("TrueClass");

    this->load(False);
    this->load(True);

    for(auto& extension : extensions) {
        extension->load_in_vm(this);
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