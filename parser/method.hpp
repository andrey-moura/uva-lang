#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include <lexer/cursor.hpp>

namespace uva {
    namespace lang {
        class object;
        enum class method_storage_type {
            instance_method,
            class_method,
        };
        struct Method
        {
            std::string name;
            std::string block;
            uva::lang::lexer::cursor block_cursor;
            method_storage_type storage_type;
            std::vector<std::string> params;
            std::function<std::shared_ptr<uva::lang::object>(uva::lang::object* object, const var& params)> function;

            Method() = default;

            Method(const std::string& __name, method_storage_type __storage_type, std::vector<std::string> params, std::string __block)
                : name(__name), block(std::move(__block)), storage_type(__storage_type) {

            };

            Method(const std::string& name, method_storage_type __storage_type, std::vector<std::string> params, std::function<std::shared_ptr<uva::lang::object>(uva::lang::object* object, const var& params)> fn)
                : name(name), function(fn), storage_type(__storage_type), params(std::move(params)) {

            }

            std::shared_ptr<uva::lang::object> call(uva::lang::object* o);
            std::shared_ptr<uva::lang::object> call(uva::lang::structure* c);
        };
    }
}