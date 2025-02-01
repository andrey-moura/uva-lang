#include <andy/lang/lang.hpp>

std::shared_ptr<andy::lang::structure> andy::lang::false_class::create()
{
    auto FalseClass = std::make_shared<andy::lang::structure>("FalseClass");

    FalseClass->methods = {
        {"present?", andy::lang::method("present?", method_storage_type::instance_method, [FalseClass](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            return std::make_shared<andy::lang::object>(FalseClass);
        })},
        {"||", andy::lang::method("||", method_storage_type::instance_method, {"other"}, [FalseClass](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            return params[0]->cls->methods["present?"].function(params[0], {}, {});
        })},
    };
    
    return FalseClass;
}