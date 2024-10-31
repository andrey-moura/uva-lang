#include <lang/lang.hpp>

#include <iostream>

#include <interpreter/interpreter.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::std_class::create(uva::lang::interpreter* interpreter)
{
    auto StdClass = std::make_shared<uva::lang::structure>("StdClass");

    StdClass->methods = {
        { "print", uva::lang::method("print", method_storage_type::class_method, {"message"}, [interpreter](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::shared_ptr<uva::lang::object> obj = params[0]->cls->methods["to_s"].call(params[0].get());
            std::cout << *obj->as<std::string>();

            return nullptr;
        })},

        { "puts", uva::lang::method("puts", method_storage_type::class_method, {"message"}, [interpreter](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::shared_ptr<uva::lang::object> obj = params[0]->cls->methods["to_s"].call(params[0].get());
            std::cout << *obj->as<std::string>() << std::endl;

            return nullptr;
        })},

        { "system", uva::lang::method("system", method_storage_type::class_method, {"command"}, [interpreter](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::shared_ptr<uva::lang::object> command = params[0]->cls->methods["to_s"].call(params[0].get());
            int code = ((std::system(command->as<std::string>()->c_str())) & 0xff00) >> 8;

            std::shared_ptr<uva::lang::object> obj = std::make_shared<uva::lang::object>(interpreter->IntegerClass);

            obj->native = new int();
            *((int*)obj->native) = code;

            return obj;
        })},
    };
    
    return StdClass;
}