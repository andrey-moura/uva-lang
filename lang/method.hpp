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
            {
                name.reserve(__name.size());
                std::string_view name_view(__name);
                while(name_view.size() && !name_view.starts_with(':')) {
                    name.push_back(name_view.front());
                    name_view.remove_prefix(1);
                }
                if(name_view.starts_with(':')) {
                    name_view.remove_prefix(1);
                    named = true;

                    if(name_view.size()) {
                        has_default_value = true;
                        std::string default_value_str = std::string(name_view);
                        default_value = var(std::move(default_value_str));
                    }
                }
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
            std::function<std::shared_ptr<uva::lang::object>(std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params)> function;

            method() = default;

            method(const std::string& __name, method_storage_type __storage_type, std::vector<std::string> __params, uva::lang::parser::ast_node __block)
                : name(__name), block_ast(std::move(__block)), storage_type(__storage_type) {
                init_params(__params);
            };

            method(const std::string& name, method_storage_type __storage_type, std::vector<std::string> __params, std::function<std::shared_ptr<uva::lang::object>(std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params)> fn)
                : name(name), function(fn), storage_type(__storage_type) {
                init_params(__params);
            }

            method(const std::string& name, method_storage_type __storage_type, std::function<std::shared_ptr<uva::lang::object>(std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params)> fn)
                : name(name), function(fn), storage_type(__storage_type) {

            }

            std::shared_ptr<uva::lang::object> call(std::shared_ptr<uva::lang::object> o);
            std::shared_ptr<uva::lang::object> call(uva::lang::structure* c);

            protected:
                void init_params(std::vector<std::string> __params);
        };
    }
}