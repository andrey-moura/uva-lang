#pragma once
#include <andy/lang/class.hpp>
#include <andy/lang/method.hpp>
#include <andy/lang/object.hpp>

namespace andy
{
    namespace lang
    {
        class interpreter;
        class float_class
        {
        public:
            static std::shared_ptr<structure> create(interpreter* interpreter);
        };
    };
};