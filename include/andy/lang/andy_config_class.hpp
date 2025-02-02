#pragma once

#include <andy/lang/class.hpp>
#include <andy/lang/method.hpp>
#include <andy/lang/object.hpp>

namespace andy
{
    namespace lang
    {
        namespace andy_config_class
        {
            std::shared_ptr<structure> create(interpreter* interpreter);
        }
    }
};