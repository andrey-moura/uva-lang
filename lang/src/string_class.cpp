#include <lang/lang.hpp>

#include <interpreter/interpreter.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::string_class::create(uva::lang::interpreter* interpreter)
{
    auto StringClass = std::make_shared<uva::lang::structure>("StringClass");

    StringClass->methods = {
        {"is_present", uva::lang::method("is_present", method_storage_type::instance_method, {}, [interpreter, StringClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string& value = object->as<std::string>();

            if(value.empty()) {
                return std::make_shared<uva::lang::object>(interpreter->FalseClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->TrueClass);
        })},

        {"to_s", uva::lang::method("to_s", method_storage_type::instance_method, {}, [interpreter, StringClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string& value = object->as<std::string>();
            return uva::lang::object::instantiate(StringClass, value);
        })},

        {"find", uva::lang::method("find", method_storage_type::instance_method, {"what"}, [interpreter, StringClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string& value = object->as<std::string>();
            size_t pos = value.find(params[0]->as<std::string>());
            return uva::lang::object::instantiate(interpreter->IntegerClass, (int32_t)pos);
        })},

        {"substr", uva::lang::method("substr", method_storage_type::instance_method, {"start", "size"}, [interpreter, StringClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string& value = object->as<std::string>();
            size_t start = params[0]->as<int32_t>();
            size_t size = params[1]->as<int32_t>();

            return uva::lang::object::instantiate(StringClass, value.substr(start, size));
        })},

        {"to_lower_case!", uva::lang::method("to_lower_case!", method_storage_type::instance_method, {}, [interpreter, StringClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::string& value = object->as<std::string>();

            for(char & c : value) {
                c = std::tolower(c);
            }

            return nullptr;
        })},

        {"to_lower_case", uva::lang::method("to_lower_case!", method_storage_type::instance_method, {}, [interpreter, StringClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::string value = object->as<std::string>();

            for(char & c : value) {
                c = std::tolower(c);
            }

            return uva::lang::object::instantiate(StringClass, value);
        })},

        {"erase!", uva::lang::method("erase!", method_storage_type::instance_method, {"start", "size"}, [interpreter, StringClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::string& value = object->as<std::string>();
            size_t start = params[0]->as<int32_t>();
            size_t size = params[1]->as<int32_t>();

            value.erase(start, size);

            return nullptr;
        })},

        {"starts_with?", uva::lang::method("starts_with?", method_storage_type::instance_method, {"what"}, [interpreter, StringClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::string& value = object->as<std::string>();
            const std::string& what = params[0]->as<std::string>();

            bool starts = value.starts_with(what);

            if(starts) {
                return std::make_shared<uva::lang::object>(interpreter->TrueClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->FalseClass);
        })},

        {"==", uva::lang::method("==", method_storage_type::instance_method, {"other"}, [interpreter, StringClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string& value = object->as<std::string>();
            const std::string& other = params[0]->as<std::string>();

            if(value == other) {
                return std::make_shared<uva::lang::object>(interpreter->TrueClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->FalseClass);
        })},

        {"!=", uva::lang::method("!=", method_storage_type::instance_method, {"other"}, [interpreter, StringClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string& value = object->as<std::string>();
            const std::string& other = params[0]->as<std::string>();

            if(value != other) {
                return std::make_shared<uva::lang::object>(interpreter->TrueClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->FalseClass);
        })},
        
        {"+", uva::lang::method("+", method_storage_type::instance_method, {"other"}, [interpreter, StringClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string& value = object->as<std::string>();
            const std::string& other = params[0]->as<std::string>();

            return uva::lang::object::instantiate(StringClass, value + other);
        })},

        {"size", uva::lang::method("size", method_storage_type::instance_method, {}, [interpreter, StringClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string& value = object->as<std::string>();
            return uva::lang::object::instantiate(interpreter->IntegerClass, (int32_t)value.size());
        })},

        {"empty?", uva::lang::method("empty?", method_storage_type::instance_method, {}, [interpreter, StringClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string& value = object->as<std::string>();

            if(value.empty()) {
                return std::make_shared<uva::lang::object>(interpreter->TrueClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->FalseClass);
        })},

        {"capitalize!", uva::lang::method("capitalize!", method_storage_type::instance_method, {}, [interpreter, StringClass](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::string& value = object->as<std::string>();

            if(!value.empty()) {
                value[0] = std::toupper(value[0]);
            }

            return nullptr;
        })},
    };
    
    return StringClass;
}