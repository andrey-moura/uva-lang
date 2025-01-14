#include <lang/lang.hpp>
#include <lang/add_operators.hpp>
#include <interpreter/interpreter.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::double_class::create(interpreter* interpreter)
{
    std::shared_ptr<uva::lang::structure> DoubleClass = std::make_shared<uva::lang::structure>("DoubleClass");
    DoubleClass->object_to_var = [](std::shared_ptr<const uva::lang::object> obj) {
        return var(obj->as<double>());
    };

    DoubleClass->methods = {
        {"present?", uva::lang::method("present?", method_storage_type::instance_method, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            double i = object->as<double>();
            
            if(i == 0) {
                return std::make_shared<uva::lang::object>(interpreter->FalseClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->TrueClass);
        })},
        {"to_string", uva::lang::method("to_string", method_storage_type::instance_method, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            double value = object->as<double>();

            return uva::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(std::to_string(value)));
        })}
    };

    uva::lang::add_operators<double>(DoubleClass, interpreter);

    return DoubleClass;
}