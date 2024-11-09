#pragma once
#include <lang/class.hpp>
#include <lang/method.hpp>
#include <lang/object.hpp>

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