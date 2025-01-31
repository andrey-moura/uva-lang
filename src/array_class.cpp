#include <uva/lang/lang.hpp>

#include <uva/lang/interpreter.hpp>
#include <uva/lang/array_class.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::array_class::create(uva::lang::interpreter* interpreter)
{
    auto ArrayClass = std::make_shared<uva::lang::structure>("Array");
    ArrayClass->object_to_var = [](std::shared_ptr<const uva::lang::object> obj) {
        std::vector<std::shared_ptr<uva::lang::object>> objects = obj->as<std::vector<std::shared_ptr<uva::lang::object>>>();

        var::array_type result;
        result.reserve(objects.size());

        for(auto& obj : objects) {
            result.push_back(obj->to_var());
        }

        return var(std::move(result));
    };

    ArrayClass->methods = {
        {"to_string", uva::lang::method("to_string", method_storage_type::instance_method, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::string result = "[";

            std::vector<std::shared_ptr<uva::lang::object>>& items = object->as<std::vector<std::shared_ptr<uva::lang::object>>>();

            for(auto& item : items) {
                if(result.size() > 1) {
                    result += ", ";
                }

                result += item->cls->methods["to_string"].call(item)->as<std::string>();
            }

            result += "]";

            return uva::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(result));
        })},

        {"join", uva::lang::method("join", method_storage_type::instance_method, {"separator"}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string& separator = params[0]->as<std::string>();
            std::string result;

            std::vector<std::shared_ptr<uva::lang::object>>& items = object->as<std::vector<std::shared_ptr<uva::lang::object>>>();

            for(auto& item : items) {
                if(result.size()) {
                    result += separator;
                }

                result += item->cls->methods["to_string"].call(item)->as<std::string>();
            }

            return uva::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(result));
        })},

        {"front", uva::lang::method("front", method_storage_type::instance_method, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::vector<std::shared_ptr<uva::lang::object>>& items = object->as<std::vector<std::shared_ptr<uva::lang::object>>>();

            if(items.empty()) {
                return std::make_shared<uva::lang::object>(interpreter->NullClass);
            }

            return items.front();
        })},

        {"size", uva::lang::method("size", method_storage_type::instance_method, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::vector<std::shared_ptr<uva::lang::object>>& items = object->as<std::vector<std::shared_ptr<uva::lang::object>>>();

            return uva::lang::object::instantiate(interpreter, interpreter->IntegerClass, items.size());
        })},

        {"pop_front!", uva::lang::method("pop_front!", method_storage_type::instance_method, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::vector<std::shared_ptr<uva::lang::object>>& items = object->as<std::vector<std::shared_ptr<uva::lang::object>>>();

            if(items.size()) {
                items.erase(items.begin());
            }

            return nullptr;
        })},
    };
    
    return ArrayClass;
}