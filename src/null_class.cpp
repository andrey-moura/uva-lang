#include <andy/lang/lang.hpp>

#include <andy/lang/interpreter.hpp>

std::shared_ptr<andy::lang::structure> andy::lang::null_class::create(andy::lang::interpreter* interpreter)
{
    auto NullClass = std::make_shared<andy::lang::structure>("NullClass");

    NullClass->methods = {
        {"present?", andy::lang::method("present?", method_storage_type::instance_method, [interpreter, NullClass](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            return std::make_shared<andy::lang::object>( interpreter->FalseClass );
        })},
        {"to_string", andy::lang::method("to_string", method_storage_type::instance_method, [interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            std::string str = "null";
            return andy::lang::object::instantiate( interpreter, interpreter->StringClass, std::move(str) );
        })},
    };
    
    return NullClass;
}