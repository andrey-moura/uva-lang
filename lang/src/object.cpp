#include "object.hpp"

#include <lang/class.hpp>
#include <lang/method.hpp>
#include <interpreter/interpreter.hpp>

#include <console.hpp>

uva::lang::object::object(std::shared_ptr<uva::lang::structure> c)
    : cls(c)
{
    if(cls) {
        uva::console::log_debug("{}#{} created", cls->name, (void*)this);
    }
}

uva::lang::object::~object()
{
    if(cls) {
        if(native_destructor) {
            uva::console::log_debug("{}#{} native destructor", cls->name, (void*)this);
            native_destructor(this);
        }

        uva::console::log_debug("{}#{} destroyed", cls->name, (void*)this);
    }
}

void uva::lang::object::initialize(uva::lang::interpreter *interpreter)
{
    for(auto& instance_variable : cls->instance_variables) {
        instance_variables[instance_variable.first] = uva::lang::object::instantiate(interpreter, instance_variable.second, nullptr);
    }

    instance_variables["this"] = shared_from_this();

    if(cls->base) {
        base_instance = uva::lang::object::instantiate(interpreter, cls->base, nullptr);
        //base_instance->initialize(interpreter);
    }

    auto new_it = cls->methods.find("new");

    if(new_it != cls->methods.end()) {
        auto new_it = cls->methods.find("new");

        if(new_it != cls->methods.end()) {
            interpreter->call(cls, shared_from_this(), new_it->second, {});
        }
    }
}

bool uva::lang::object::is_present() const
{
    if(!cls) {
        throw std::runtime_error("object has no class");
    }
    
    auto it = cls->methods.find("is_present");

    if(it == cls->methods.end()) {
        throw std::runtime_error("is_present is not defined in class " + cls->name);
    } else {
        auto this_without_const = const_cast<object*>(this);

        auto obj = it->second.call( this_without_const->shared_from_this() );

        if(obj->cls->name == "TrueClass") {
            return true;
        } else {
            return false;
        }
    }
}