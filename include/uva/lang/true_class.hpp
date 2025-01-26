#pragma once

#include <uva/lang/class.hpp>
#include <uva/lang/method.hpp>
#include <uva/lang/object.hpp>

namespace uva
{
    namespace lang
    {
        namespace true_class
        {
            std::shared_ptr<structure> create(uva::lang::interpreter* interpreter);
        }
    }
};