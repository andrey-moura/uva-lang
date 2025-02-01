#include <andy/lang/lang.hpp>
#include "add_operators.hpp"
#include <andy/lang/interpreter.hpp>

std::shared_ptr<andy::lang::structure> andy::lang::double_class::create(interpreter* interpreter)
{
    std::shared_ptr<andy::lang::structure> DoubleClass = std::make_shared<andy::lang::structure>("DoubleClass");
    DoubleClass->object_to_var = [](std::shared_ptr<const andy::lang::object> obj) {
        return var(obj->as<double>());
    };

    DoubleClass->methods = {
        {"present?", andy::lang::method("present?", method_storage_type::instance_method, [interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            double i = object->as<double>();
            
            if(i == 0) {
                return std::make_shared<andy::lang::object>(interpreter->FalseClass);
            }

            return std::make_shared<andy::lang::object>(interpreter->TrueClass);
        })},
        {"to_string", andy::lang::method("to_string", method_storage_type::instance_method, [interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            double value = object->as<double>();

            return andy::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(std::to_string(value)));
        })}
    };

    andy::lang::add_operators<double>(DoubleClass, interpreter);

    return DoubleClass;
}