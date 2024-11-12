#pragma once

#include <lang/class.hpp>
#include <lang/method.hpp>
#include <lang/object.hpp>

namespace uva
{
    namespace lang
    {
        class interpreter;
        namespace null_class
        {
            std::shared_ptr<structure> create(uva::lang::interpreter* interpreter);
        }
    }
};