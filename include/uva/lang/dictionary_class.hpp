#pragma once
#include <uva/lang/class.hpp>
#include <uva/lang/method.hpp>
#include <uva/lang/object.hpp>

namespace uva
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