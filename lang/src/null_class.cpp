#include <lang/lang.hpp>

#include <interpreter/interpreter.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::null_class::create(uva::lang::interpreter* interpreter)
{
    auto NullClass = std::make_shared<uva::lang::structure>("NullClass");

    NullClass->methods = {
        {"present?", uva::lang::method("present?", method_storage_type::instance_method, {}, [interpreter, NullClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            return std::make_shared<uva::lang::object>( interpreter->FalseClass );
        })},
        {"to_s", uva::lang::method("to_s", method_storage_type::instance_method, {}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::string str = "null";
            return uva::lang::object::instantiate( interpreter, interpreter->StringClass, std::move(str) );
        })},
    };
    
    return NullClass;
}