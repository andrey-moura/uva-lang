#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <uva/var.hpp>

#include <lang/method.hpp>

namespace uva {
    namespace lang {
        class object;
        class method;
        class structure
        {
        public:
            //for user code, use create
            structure(const std::string& __name, std::vector<uva::lang::method> __methods = {});
            ~structure();
        public:
            std::string name;
            std::map<std::string, uva::lang::method> methods;
            std::shared_ptr<uva::lang::structure> base;
            std::map<std::string, std::shared_ptr<uva::lang::object>> instance_variables;
            std::string source_content;

            std::shared_ptr<uva::lang::object> call(const uva::lang::method& method, const var& params= null);
            std::shared_ptr<uva::lang::object> call(const std::string& method, const var& params = null)
            {
                return call(methods[method], params);
            }
        };
    };
};