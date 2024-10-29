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
        class method
        {
        public:
            std::string name;
            std::string block;
            uva::lang::parser::ast_node block_ast;
            method_storage_type storage_type;
            std::vector<std::string> params;
            std::function<std::shared_ptr<uva::lang::object>(uva::lang::object* object, const var& params)> function;

            method() = default;

            method(const std::string& __name, method_storage_type __storage_type, std::vector<std::string> params, uva::lang::parser::ast_node __block)
                : name(__name), block_ast(std::move(__block)), storage_type(__storage_type) {

            };

            method(const std::string& name, method_storage_type __storage_type, std::vector<std::string> params, std::function<std::shared_ptr<uva::lang::object>(uva::lang::object* object, const var& params)> fn)
                : name(name), function(fn), storage_type(__storage_type), params(std::move(params)) {

            }

            std::shared_ptr<uva::lang::object> call(uva::lang::object* o);
            std::shared_ptr<uva::lang::object> call(uva::lang::structure* c);
        };
    }
}