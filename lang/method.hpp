#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include <parser/parser.hpp>

namespace uva {
    namespace lang {
        class object;
        class structure;
        enum class method_storage_type {
            instance_method,
            class_method,
        };
        struct fn_parameter
        {
            fn_parameter() = default;
            fn_parameter(std::string __name)
                : name(std::move(__name))
            {

            }
            fn_parameter(std::string __name, bool __named, var __default_value)
                : name(std::move(__name)), named(__named), default_value(std::move(__default_value)), has_default_value(true) {
            }
            std::string name;
            bool has_default_value = false;
            var default_value;
            bool named = false;
        };
        class method
        {
        public:
            std::string name;
            std::string block;
            uva::lang::parser::ast_node block_ast;
            method_storage_type storage_type;
            std::vector<fn_parameter> positional_params;
            std::vector<fn_parameter> named_params;
            std::function<std::shared_ptr<uva::lang::object>(std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> position_params, std::map<std::string, std::shared_ptr<uva::lang::object>> named_params)> function;

            method() = default;

            method(const std::string& __name, method_storage_type __storage_type, std::vector<std::string> __params, uva::lang::parser::ast_node __block)
                : name(__name), block_ast(std::move(__block)), storage_type(__storage_type) {
                init_params(__params);
            };

            method(const std::string& name, method_storage_type __storage_type, std::initializer_list<std::string> __params, std::function<std::shared_ptr<uva::lang::object>(std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params)> fn)
                : name(name), storage_type(__storage_type) {
                init_params(__params);
                function = [fn](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> positional_params, std::map<std::string, std::shared_ptr<uva::lang::object>> named_params) {
                    return fn(object, positional_params);
                };
            }

            method(const std::string& name, method_storage_type __storage_type, std::vector<fn_parameter> __params, std::function<std::shared_ptr<uva::lang::object>(std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> positional_params, std::map<std::string, std::shared_ptr<uva::lang::object>> named_params)> fn)
                : name(name), function(fn), storage_type(__storage_type) {
                positional_params.reserve(__params.size());
                named_params.reserve(__params.size());

                for(auto& param : __params) {
                    if(param.named) {
                        named_params.push_back(std::move(param));
                    } else {
                        positional_params.push_back(std::move(param));
                    }
                }
            }

            method(const std::string& name, method_storage_type __storage_type, std::function<std::shared_ptr<uva::lang::object>(std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params)> fn)
                : name(name), storage_type(__storage_type) {
                function = [fn](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> positional_params, std::map<std::string, std::shared_ptr<uva::lang::object>> named_params) {
                    return fn(object, positional_params);
                };
            }

            std::shared_ptr<uva::lang::object> call(std::shared_ptr<uva::lang::object> o);
            std::shared_ptr<uva::lang::object> call(uva::lang::structure* c);

            protected:
                void init_params(std::vector<std::string> __params);
        };
    }
}