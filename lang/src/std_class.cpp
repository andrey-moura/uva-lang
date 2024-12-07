#include <lang/lang.hpp>

#include <iostream>

#include <interpreter/interpreter.hpp>
#include <extension/extension.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::std_class::create(uva::lang::interpreter* interpreter)
{
    auto StdClass = std::make_shared<uva::lang::structure>("StdClass");

    StdClass->methods = {
        { "print", uva::lang::method("print", method_storage_type::class_method, {"message"}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::shared_ptr<uva::lang::object> obj = params[0]->cls->methods["to_string"].call(params[0]);
            std::cout << obj->as<std::string>();

            return nullptr;
        })},

        { "puts", uva::lang::method("puts", method_storage_type::class_method, {"message"}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::shared_ptr<uva::lang::object> obj = params[0]->cls->methods["to_string"].call(params[0]);
            std::cout << obj->as<std::string>() << std::endl;

            return nullptr;
        })},

        { "gets", uva::lang::method("gets", method_storage_type::class_method, {}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::string line;
            std::getline(std::cin, line);

            return uva::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(line));
        })},

        { "system", uva::lang::method("system", method_storage_type::class_method, {"command"}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::shared_ptr<uva::lang::object> command = params[0]->cls->methods["to_string"].call(params[0]);
            int code = ((std::system(command->as<std::string>().c_str())) & 0xff00) >> 8;

            return uva::lang::object::instantiate(interpreter, interpreter->IntegerClass, code);
        })},

        { "import", uva::lang::method("import", method_storage_type::class_method, {"module"}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::string module = params[0]->as<std::string>();
            extension::import(interpreter, module);
            return nullptr;
        })},
    };
    
    return StdClass;
}