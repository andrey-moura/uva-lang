#include <lang/lang.hpp>

#include <interpreter/interpreter.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::array_class::create(uva::lang::interpreter* interpreter)
{
    auto ArrayClass = std::make_shared<uva::lang::structure>("Array");

    ArrayClass->methods = {
        {"join", uva::lang::method("join", method_storage_type::instance_method, {"separator"}, [interpreter](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::shared_ptr<uva::lang::object> obj = std::make_shared<uva::lang::object>(interpreter->StringClass);
            
            std::string* separator = params[0]->as<std::string>();
            std::string* result = new std::string();

            std::vector<std::shared_ptr<uva::lang::object>>& items = *(object->as<std::vector<std::shared_ptr<uva::lang::object>>>());

            for(auto& item : items) {
                if(result->size()) {
                    *result += *separator;
                }
                *result += *item->cls->methods["to_s"].call(item.get())->as<std::string>();
            }

            obj->native = result;

            return obj;
        })}
    };
    
    return ArrayClass;
}