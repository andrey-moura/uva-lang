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
                if(name.ends_with(':')) {
                    named = true;
                    name.pop_back();
                }
            }
            std::string name;
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