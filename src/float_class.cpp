#include <uva/lang/lang.hpp>
#include "add_operators.hpp"
#include <uva/lang/interpreter.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::float_class::create(interpreter* interpreter)
{
    std::shared_ptr<uva::lang::structure> FloatClass = std::make_shared<uva::lang::structure>("FloatClass");
    FloatClass->object_to_var = [](std::shared_ptr<const uva::lang::object> obj) {
        return var(obj->as<float>());
    };

    FloatClass->methods = {
        {"present?", uva::lang::method("present?", method_storage_type::instance_method, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            float i = object->as<float>();
            
            if(i == 0) {
                return std::make_shared<uva::lang::object>(interpreter->FalseClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->TrueClass);
        })},
        {"to_string", uva::lang::method("to_string", method_storage_type::instance_method, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            float value = object->as<float>();

            return uva::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(std::to_string(value)));
        })}
    };

    uva::lang::add_operators<float>(FloatClass, interpreter);

    return FloatClass;
}