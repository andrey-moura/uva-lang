#pragma once
#include <lang/class.hpp>
#include <lang/method.hpp>
#include <lang/object.hpp>

namespace uva
{
    namespace lang
    {
        class interpreter;
        class double_class
        {
        public:
            static std::shared_ptr<structure> create(interpreter* interpreter);
        };
    };
};