#include "class.hpp"

#include <console.hpp>

#include "false_class.cpp"
#include "true_class.cpp"
#include "string_class.cpp"
#include "integer_class.cpp"
#include "double_class.cpp"
#include "float_class.cpp"
#include "file_class.cpp"
#include "std_class.cpp"
#include "array_class.cpp"
#include "null_class.cpp"
#include "dictionary_class.cpp"

uva::lang::structure::structure(const std::string& __name, std::vector<uva::lang::method> __methods)
    : name(__name)
{
    uva::console::log_debug("{}#Class created", name);
}

uva::lang::structure::~structure()
{
    uva::console::log_debug("{}#Class destroyed", name);
}