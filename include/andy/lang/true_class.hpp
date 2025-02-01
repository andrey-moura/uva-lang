#pragma once

#include <andy/lang/class.hpp>
#include <andy/lang/method.hpp>
#include <andy/lang/object.hpp>

namespace andy
{
    namespace lang
    {
        namespace true_class
        {
            std::shared_ptr<structure> create(andy::lang::interpreter* interpreter);
        }
    }
};