#include <lang/lang.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::false_class::create()
{
    auto FalseClass = std::make_shared<uva::lang::structure>("FalseClass");

    FalseClass->methods = {
        {"present?", uva::lang::method("present?", method_storage_type::instance_method, [FalseClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            return std::make_shared<uva::lang::object>(FalseClass);
        })},
        {"||", uva::lang::method("||", method_storage_type::instance_method, {"other"}, [FalseClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            return params[0]->cls->methods["present?"].function(params[0], {});
        })},
    };
    
    return FalseClass;
}