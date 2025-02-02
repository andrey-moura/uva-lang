#include <andy/lang/lang.hpp>

#include <andy/lang/interpreter.hpp>
#include <andy/lang/extension.hpp>

#include <filesystem>
#include <iostream>

std::shared_ptr<andy::lang::structure> andy::lang::path_class::create(andy::lang::interpreter* interpreter)
{
    auto PathClass = std::make_shared<andy::lang::structure>("Path");
    PathClass->methods = {
        {"new", andy::lang::method("new", method_storage_type::instance_method, {"path"}, [interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            object->set_native<std::filesystem::path>(std::move(std::filesystem::path(params[0]->as<std::string>())));

            return nullptr;
        })},
        {"to_string", andy::lang::method("to_string", method_storage_type::instance_method, [interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            return andy::lang::object::instantiate(interpreter, interpreter->StringClass, object->as<std::filesystem::path>().string());
        })},
        {"exists?", andy::lang::method("exists?", method_storage_type::instance_method, [interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            std::filesystem::path& path = object->as<std::filesystem::path>();

            if(std::filesystem::exists(path)) {
                return std::make_shared<andy::lang::object>(interpreter->TrueClass);
            }

            return std::make_shared<andy::lang::object>(interpreter->FalseClass);
        })},
        {"/=", andy::lang::method("/=", method_storage_type::instance_method, {"path"}, [interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            std::filesystem::path& path = object->as<std::filesystem::path>();
            path /= params[0]->as<std::string>();

            return nullptr;
        })},
        {"/", andy::lang::method("/", method_storage_type::instance_method, {"path"}, [interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            std::filesystem::path path = object->as<std::filesystem::path>() / params[0]->as<std::string>();
            
            return andy::lang::object::create(interpreter, interpreter->PathClass, std::move(path));
        })},
    };
    
    return PathClass;
}