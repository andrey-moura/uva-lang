#include <andy/lang/lang.hpp>

#include <andy/lang/interpreter.hpp>

std::shared_ptr<andy::lang::structure> andy::lang::true_class::create(andy::lang::interpreter* interpreter)
{
    auto TrueClass = std::make_shared<andy::lang::structure>("TrueClass");

    TrueClass->methods = {
        {"present?", andy::lang::method("present?", method_storage_type::instance_method, [TrueClass](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            return std::make_shared<andy::lang::object>(TrueClass);
        })},
        {"||", andy::lang::method("||", method_storage_type::instance_method, {"other"}, [TrueClass](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            return std::make_shared<andy::lang::object>(TrueClass);
        })},
        {"!", andy::lang::method("!", method_storage_type::instance_method, {}, [interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            return std::make_shared<andy::lang::object>(interpreter->FalseClass);
        })},
        { "&&", andy::lang::method("&&", method_storage_type::instance_method, {"other"}, [TrueClass, interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            std::shared_ptr<andy::lang::object> other = params[0];
            if(other->is_present()) {
                return std::make_shared<andy::lang::object>(TrueClass);
            } else {
                return std::make_shared<andy::lang::object>(interpreter->FalseClass);
            }
        })}, 
    };
    
    return TrueClass;
}