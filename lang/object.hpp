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
        class object
        {
        public:
            object(std::shared_ptr<uva::lang::structure> c) : cls(c) {};
            ~object();
            std::shared_ptr<uva::lang::structure> cls;
            void* native = nullptr;
            std::shared_ptr<object> base_instance = nullptr;
            object* derived_instance = nullptr;

            bool is_present() const;

            template<typename T>
            const T& as() const {
                return *static_cast<T*>(native);
            }
            template<typename T>
            T& as() {
                return *static_cast<T*>(native);
            }
        };
    };
};