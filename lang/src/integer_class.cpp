#include <lang/lang.hpp>

#include <interpreter/interpreter.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::integer_class::create(interpreter* interpreter)
{
    std::shared_ptr<uva::lang::structure> IntegerClass = std::make_shared<uva::lang::structure>("IntegerClass");

    IntegerClass->methods = {
        {"is_present", uva::lang::method("is_present", method_storage_type::instance_method, {}, [interpreter](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int i = object->as<int>();
            
            if(i == 0) {
                return std::make_shared<uva::lang::object>(interpreter->FalseClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->TrueClass);
        })},
        {"to_s", uva::lang::method("to_s", method_storage_type::instance_method, {}, [interpreter](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int value = object->as<int>();

            return interpreter->instantiate(interpreter->StringClass, std::move(std::to_string(value)));
        })}
    };

    return IntegerClass;
}