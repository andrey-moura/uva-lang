#include <lang/lang.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::true_class::create()
{
    auto TrueClass = std::make_shared<uva::lang::structure>("TrueClass");

    TrueClass->methods = {
        {"is_present", uva::lang::method("is_present", method_storage_type::instance_method, {}, [TrueClass](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            return std::make_shared<uva::lang::object>(TrueClass);
        })}
    };
    
    return TrueClass;
}