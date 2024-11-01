#include <lang/lang.hpp>

#include <interpreter/interpreter.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::string_class::create(uva::lang::interpreter* interpreter)
{
    auto StringClass = std::make_shared<uva::lang::structure>("StringClass");

    StringClass->methods = {
        {"is_present", uva::lang::method("is_present", method_storage_type::instance_method, {}, [interpreter, StringClass](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string& value = object->as<std::string>();

            if(value.empty()) {
                return std::make_shared<uva::lang::object>(interpreter->FalseClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->TrueClass);
        })},
        {"to_s", uva::lang::method("to_s", method_storage_type::instance_method, {}, [interpreter, StringClass](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string& value = object->as<std::string>();
            return interpreter->instantiate(StringClass, value);
        })}
    };
    
    return StringClass;
}