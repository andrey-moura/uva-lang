#include "method.hpp"
#include "object.hpp"
#include "class.hpp"

std::shared_ptr<uva::lang::object> uva::lang::method::call(std::shared_ptr<uva::lang::object> o)
{
    return function(o, std::vector<std::shared_ptr<uva::lang::object>>());
}