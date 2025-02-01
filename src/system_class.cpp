#include <andy/lang/lang.hpp>

#include <andy/lang/interpreter.hpp>
#include <andy/lang/extension.hpp>

std::shared_ptr<andy::lang::structure> andy::lang::system_class::create(andy::lang::interpreter* interpreter)
{
    auto SystemClass = std::make_shared<andy::lang::structure>("System");
    SystemClass->methods = {
        {"Windows?", andy::lang::method("Windows?", method_storage_type::instance_method, [interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            return std::make_shared<andy::lang::object>(interpreter->FalseClass);
        })},
        {"Linux?", andy::lang::method("Linux?", method_storage_type::instance_method, [interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            return std::make_shared<andy::lang::object>(interpreter->FalseClass);
        })},
    };
#ifdef _WIN32
        SystemClass->class_variables["OS"] = andy::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(std::string("Windows")));
        SystemClass->methods["Windows?"] = andy::lang::method("Windows?", method_storage_type::instance_method, [interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            return std::make_shared<andy::lang::object>(interpreter->TrueClass);
        });
#elif __linux__
        SystemClass->class_variables["OS"] = andy::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(std::string("Linux")));
        SystemClass->methods["Linux?"] = andy::lang::method("Linux?", method_storage_type::instance_method, [interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            return std::make_shared<andy::lang::object>(interpreter->TrueClass);
        });
#else
        throw std::runtime_error("unsupported OS");
#endif
    
    return SystemClass;
}