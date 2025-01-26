#pragma once
#include <uva/lang/class.hpp>
#include <uva/lang/method.hpp>
#include <uva/lang/object.hpp>

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