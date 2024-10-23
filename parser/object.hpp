#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace uva
{
    namespace lang {
        class object;
        class structure;
        struct object
        {
            object(std::shared_ptr<uva::lang::structure> c) : cls(c) {};
            ~object();
            std::shared_ptr<uva::lang::structure> cls;
            void* native = nullptr;
            std::shared_ptr<object> base_instance = nullptr;
            object* derived_instance = nullptr;
        };
    };
};