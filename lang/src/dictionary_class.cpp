#include <lang/lang.hpp>

#include <interpreter/interpreter.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::dictionary_class::create(uva::lang::interpreter* interpreter)
{
    auto DictionaryClass = std::make_shared<uva::lang::structure>("Dictionary");
    DictionaryClass->object_to_var = [](std::shared_ptr<const uva::lang::object> obj) {
        uva::lang::dictionary data = obj->as<uva::lang::dictionary>();

        var::dictionary_type result;

        for(auto& pair : data) {
            result[pair.first->to_var()] = pair.second->to_var();
        }

        return var(std::move(result));
    };
    DictionaryClass->methods = {
        {"present?", uva::lang::method("present?", method_storage_type::instance_method, {}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
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