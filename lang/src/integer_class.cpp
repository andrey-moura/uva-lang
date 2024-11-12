#include <lang/lang.hpp>

#include <interpreter/interpreter.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::integer_class::create(interpreter* interpreter)
{
    std::shared_ptr<uva::lang::structure> IntegerClass = std::make_shared<uva::lang::structure>("IntegerClass");
    IntegerClass->object_to_var = [](std::shared_ptr<const uva::lang::object> obj) {
        return var(obj->as<int>());
    };

    IntegerClass->methods = {
        {"present?", uva::lang::method("present?", method_storage_type::instance_method, {}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int i = object->as<int>();
            
            if(i == 0) {
                return std::make_shared<uva::lang::object>(interpreter->FalseClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->TrueClass);
        })},
        {"to_s", uva::lang::method("to_s", method_storage_type::instance_method, {}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int value = object->as<int>();

            return uva::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(std::to_string(value)));
        })},
        {"+", uva::lang::method("+", method_storage_type::instance_method, {"other"}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int value = object->as<int>();
            int other = params[0]->as<int>();

            return uva::lang::object::instantiate(interpreter, interpreter->IntegerClass, value + other);
        })},
        {"-", uva::lang::method("-", method_storage_type::instance_method, {"other"}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int value = object->as<int>();
            int other = params[0]->as<int>();

            return uva::lang::object::instantiate(interpreter, interpreter->IntegerClass, value - other);
        })},
        {"*", uva::lang::method("*", method_storage_type::instance_method, {"other"}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int value = object->as<int>();
            int other = params[0]->as<int>();

            return uva::lang::object::instantiate(interpreter, interpreter->IntegerClass, value * other);
        })},
        {"/", uva::lang::method("/", method_storage_type::instance_method, {"other"}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int value = object->as<int>();
            int other = params[0]->as<int>();

            return uva::lang::object::instantiate(interpreter, interpreter->IntegerClass, value / other);
        })},
        {"%", uva::lang::method("%", method_storage_type::instance_method, {"other"}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int value = object->as<int>();
            int other = params[0]->as<int>();

            return uva::lang::object::instantiate(interpreter, interpreter->IntegerClass, value % other);
        })},
        {"!=", uva::lang::method("!=", method_storage_type::instance_method, {"other"}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int value = object->as<int>();
            int other = params[0]->as<int>();

            if(value != other) {
                return std::make_shared<uva::lang::object>(interpreter->TrueClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->FalseClass);
        })},
        {"==", uva::lang::method("==", method_storage_type::instance_method, {"other"}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int value = object->as<int>();
            int other = params[0]->as<int>();

            if(value == other) {
                return std::make_shared<uva::lang::object>(interpreter->TrueClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->FalseClass);
        })},
        {"<", uva::lang::method("<", method_storage_type::instance_method, {"other"}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int value = object->as<int>();
            int other = params[0]->as<int>();

            if(value < other) {
                return std::make_shared<uva::lang::object>(interpreter->TrueClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->FalseClass);
        })},
        {">", uva::lang::method(">", method_storage_type::instance_method, {"other"}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            int value = object->as<int>();
            int other = params[0]->as<int>();

            if(value > other) {
                return std::make_shared<uva::lang::object>(interpreter->TrueClass);
            }

            return std::make_shared<uva::lang::object>(interpreter->FalseClass);
        })},
    };

    return IntegerClass;
}