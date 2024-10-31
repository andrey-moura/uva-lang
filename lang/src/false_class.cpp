#include <lang/lang.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::false_class::create()
{
    auto FalseClass = std::make_shared<uva::lang::structure>("FalseClass");

    FalseClass->methods = {
        {"is_present", uva::lang::method("is_present", method_storage_type::instance_method, {}, [FalseClass](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            return std::make_shared<uva::lang::object>(FalseClass);
        })}
    };
    
    return FalseClass;
}