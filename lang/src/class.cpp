#include "class.hpp"

#include <console.hpp>

uva::lang::structure::structure(const std::string& __name, std::vector<uva::lang::method> __methods)
    : name(__name)
{
    uva::console::log_debug("{}#Class created", name);
}

uva::lang::structure::~structure()
{
    uva::console::log_debug("{}#Class destroyed", name);
}