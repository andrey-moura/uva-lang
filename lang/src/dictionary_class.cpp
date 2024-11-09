#include <lang/lang.hpp>

#include <interpreter/interpreter.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::dictionary_class::create(uva::lang::interpreter* interpreter)
{
    auto DictionaryClass = std::make_shared<uva::lang::structure>("Dictionary");

    DictionaryClass->methods = {
        {"is_present", uva::lang::method("is_present", method_storage_type::instance_method, {}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string& value = object->as<std::string>();

            if(value.empty()) {
                return std::make_shared<uva::lang::object>(interpreter->FalseClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->TrueClass);
        })},
        {"[]", uva::lang::method("[]", method_storage_type::instance_method, {"key"}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::shared_ptr<uva::lang::object> key = params[0];

            auto& dictionary = object->as<uva::lang::dictionary>();

            auto operator_it = key->cls->methods.find("==");

            for(auto& pair : dictionary) {
                auto result = interpreter->call(key->cls, key, operator_it->second, { pair.first });
                if(result->cls == interpreter->TrueClass) {
                    return pair.second;
                }
            }

            return std::make_shared<uva::lang::object>(interpreter->NullClass);
        })},
    };
    
    return DictionaryClass;
}