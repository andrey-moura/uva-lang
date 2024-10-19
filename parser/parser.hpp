#pragma once

#include <filesystem>
#include <map>
#include <functional>
#include <string>
#include <uva/core.hpp>
#include "source_cursor.hpp"

namespace uva
{
    class Object;
    class Class;
    enum class method_storage_type {
        instance_method,
        class_method,
    };
    struct Method
    {
        std::string name;
        std::string block;
        uva::lang::source_cursor block_cursor;
        method_storage_type storage_type;
        std::vector<std::string> params;
        std::function<std::shared_ptr<Object>(Object* object, const var& params)> function;

        Method() = default;

        Method(const std::string& __name, method_storage_type __storage_type, std::vector<std::string> params, std::string __block)
            : name(__name), block(std::move(__block)), storage_type(__storage_type) {

        };

        Method(const std::string& name, method_storage_type __storage_type, std::vector<std::string> params, std::function<std::shared_ptr<Object>(Object* object, const var& params)> fn)
            : name(name), function(fn), storage_type(__storage_type), params(std::move(params)) {

        }

        std::shared_ptr<Object> call(Object* o);
        std::shared_ptr<Object> call(Class* c);
    };
    class Object;
    struct Class : public std::enable_shared_from_this<Class>
    {
        //for user code, use create
        Class(const std::string& __name, std::vector<Method> __methods = {});
        ~Class();
    public:
        std::string name;
        std::map<std::string, Method> methods;
        std::shared_ptr<Class> base;
        std::map<std::string, std::shared_ptr<Object>> variables;
        std::string source_content;

        std::shared_ptr<Object> call(const Method& method, const var& params= null);
        std::shared_ptr<Object> call(const std::string& method, const var& params = null)
        {
            return call(methods[method], params);
        }
    };
    class Context : public Class
    {
    public:
        Context()
            : Class("Context")
        {

        }
    public:
        std::map<std::string, std::shared_ptr<Object>> variables;
    };
    struct Object : public std::enable_shared_from_this<Object>
    {
        Object(std::shared_ptr<Class> c) : cls(c) {};
        ~Object();
        std::shared_ptr<Class> cls;
        void* native = nullptr;
        std::shared_ptr<Object> base_instance = nullptr;
        Object* derived_instance = nullptr;
    };
    class parser
    {
    public:
        parser() = default;
        ~parser() = default;
    public:
        std::shared_ptr<Class> parse(const std::filesystem::path& path);
    };
}; // namespace uva