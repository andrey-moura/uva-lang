#include <uva/lang/lang.hpp>
#include <uva/lang/interpreter.hpp>

#include "add_operators.hpp"

std::shared_ptr<uva::lang::structure> uva::lang::integer_class::create(interpreter* interpreter)
{
    std::shared_ptr<uva::lang::structure> IntegerClass = std::make_shared<uva::lang::structure>("IntegerClass");
    IntegerClass->object_to_var = [](std::shared_ptr<const uva::lang::object> obj) {
        return var(obj->as<int>());
    };

    IntegerClass->methods = {
        {"present?", uva::lang::method("present?", method_storage_type::instance_method, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int i = object->as<int>();
            
            if(i == 0) {
                return std::make_shared<uva::lang::object>(interpreter->FalseClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->TrueClass);
        })},
        {"to_string", uva::lang::method("to_string", method_storage_type::instance_method, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int value = object->as<int>();

            return uva::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(std::to_string(value)));
        })}
    };

    uva::lang::add_operators<int>(IntegerClass, interpreter);

    return IntegerClass;
}