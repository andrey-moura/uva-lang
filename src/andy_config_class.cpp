#include <andy/lang/lang.hpp>

#include <andy/lang/interpreter.hpp>
#include <andy/lang/extension.hpp>
#include <andy/lang/config.hpp>

#include <filesystem>

std::shared_ptr<andy::lang::structure> andy::lang::andy_config_class::create(andy::lang::interpreter* interpreter)
{
    auto AndyConfigClass = std::make_shared<andy::lang::structure>("AndyConfig");

    std::filesystem::path src_dir = andy::lang::config::src_dir();

    std::shared_ptr<andy::lang::object> src_dir_obj = andy::lang::object::create(interpreter, interpreter->PathClass, std::move(src_dir));

    AndyConfigClass->class_variables["src_dir"] = src_dir_obj;
    
    return AndyConfigClass;
}