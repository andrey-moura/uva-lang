#include "object.hpp"

#include <lang/class.hpp>
#include <lang/method.hpp>

#include <console.hpp>

uva::lang::object::~object()
{
    uva::console::log_debug("{}#{} destroyed", cls->name, (void*)this);
}

bool uva::lang::object::is_present() const
{
    auto it = cls->methods.find("is_present");

    if(it == cls->methods.end()) {
        throw std::runtime_error("is_present is not defined in class " + cls->name);
    } else {
        auto this_without_const = const_cast<object*>(this);

        auto obj = it->second.call( this_without_const );

        if(obj->cls->name == "TrueClass") {
            return true;
        } else {
            return false;
        }
    }
}