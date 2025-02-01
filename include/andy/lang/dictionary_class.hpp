#pragma once
#include <andy/lang/class.hpp>
#include <andy/lang/method.hpp>
#include <andy/lang/object.hpp>

namespace andy
{
    namespace lang
    {
        class interpreter;
        using dictionary = std::vector<std::pair<std::shared_ptr<object>, std::shared_ptr<object>>>;
        class dictionary_class
        {
        public:
            static std::shared_ptr<structure> create(interpreter* interpreter);
        };
    };
};