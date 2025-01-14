#pragma once
#include <lang/class.hpp>
#include <lang/method.hpp>
#include <lang/object.hpp>

namespace uva
{
    namespace lang
    {
        template<typename T>
        void add_operators(std::shared_ptr<uva::lang::structure> cls, interpreter* interpreter)
        {
            cls->methods["+"] = uva::lang::method("+", method_storage_type::instance_method, {"other"}, [interpreter, cls](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
                T& value = object->as<T>();
                std::shared_ptr<uva::lang::object> other = params[0];
                if(other->cls == interpreter->DoubleClass) {
                    return uva::lang::object::instantiate(interpreter, interpreter->DoubleClass, value + other->as<double>());
                } else if(other->cls == interpreter->IntegerClass) {
                    return uva::lang::object::instantiate(interpreter, cls, value + other->as<int>());
                } else if(other->cls == interpreter->FloatClass) {
                    return uva::lang::object::instantiate(interpreter, interpreter->FloatClass, value + other->as<float>());
                }

                throw std::runtime_error("undefined operator+(" + object->cls->name + ", " + other->cls->name + ")");
            });
            cls->methods["-"] = uva::lang::method("-", method_storage_type::instance_method, {"other"}, [interpreter, cls](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
                T& value = object->as<T>();
                std::shared_ptr<uva::lang::object> other = params[0];
                if(other->cls == interpreter->DoubleClass) {
                    return uva::lang::object::instantiate(interpreter, interpreter->DoubleClass, value - other->as<double>());
                } else if(other->cls == interpreter->IntegerClass) {
                    return uva::lang::object::instantiate(interpreter, cls, value - other->as<int>());
                } else if(other->cls == interpreter->FloatClass) {
                    return uva::lang::object::instantiate(interpreter, interpreter->FloatClass, value - other->as<float>());
                }

                throw std::runtime_error("undefined operator-(" + object->cls->name + ", " + other->cls->name + ")");
            });
            cls->methods["*"] = uva::lang::method("*", method_storage_type::instance_method, {"other"}, [interpreter, cls](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
                T& value = object->as<T>();
                std::shared_ptr<uva::lang::object> other = params[0];
                if(other->cls == interpreter->DoubleClass) {
                    return uva::lang::object::instantiate(interpreter, interpreter->DoubleClass, value * other->as<double>());
                } else if(other->cls == interpreter->IntegerClass) {
                    return uva::lang::object::instantiate(interpreter, cls, value * other->as<int>());
                } else if(other->cls == interpreter->FloatClass) {
                    return uva::lang::object::instantiate(interpreter, interpreter->FloatClass, value * other->as<float>());
                }

                throw std::runtime_error("undefined operator*(" + object->cls->name + ", " + other->cls->name + ")");
            });
            cls->methods["/"] = uva::lang::method("/", method_storage_type::instance_method, {"other"}, [interpreter, cls](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
                T& value = object->as<T>();
                std::shared_ptr<uva::lang::object> other = params[0];
                if(other->cls == interpreter->DoubleClass) {
                    return uva::lang::object::instantiate(interpreter, interpreter->DoubleClass, value / other->as<double>());
                } else if(other->cls == interpreter->IntegerClass) {
                    return uva::lang::object::instantiate(interpreter, cls, value / other->as<int>());
                } else if(other->cls == interpreter->FloatClass) {
                    return uva::lang::object::instantiate(interpreter, interpreter->FloatClass, value / other->as<float>());
                }

                throw std::runtime_error("undefined operator/(" + object->cls->name + ", " + other->cls->name + ")");
            });
            if constexpr (std::is_integral_v<T>) {
                cls->methods["%"] = uva::lang::method("%", method_storage_type::instance_method, {"other"}, [interpreter, cls](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
                    T& value = object->as<T>();
                    std::shared_ptr<uva::lang::object> other = params[0];
                    if(other->cls == interpreter->IntegerClass) {
                        return uva::lang::object::instantiate(interpreter, cls, value % other->as<int>());
                    }

                    throw std::runtime_error("undefined operator%(" + object->cls->name + ", " + other->cls->name + ")");
                });
            }
            cls->methods["++"] = uva::lang::method("+", method_storage_type::instance_method, [interpreter, cls](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
                T& value = object->as<T>();
                value++;

                return nullptr;
            });
            cls->methods["!="] = uva::lang::method("!=", method_storage_type::instance_method, {"other"}, [interpreter, cls](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
                T& value = object->as<T>();
                std::shared_ptr<uva::lang::object> other = params[0];
                if(other->cls == interpreter->DoubleClass) {
                    return value != other->as<double>() ? std::make_shared<uva::lang::object>(interpreter->TrueClass) : std::make_shared<uva::lang::object>(interpreter->FalseClass);
                } else if(other->cls == interpreter->IntegerClass) {
                    return value != other->as<int>() ? std::make_shared<uva::lang::object>(interpreter->TrueClass) : std::make_shared<uva::lang::object>(interpreter->FalseClass);
                } else if(other->cls == interpreter->FloatClass) {
                    return value != other->as<float>() ? std::make_shared<uva::lang::object>(interpreter->TrueClass) : std::make_shared<uva::lang::object>(interpreter->FalseClass);
                }

                throw std::runtime_error("undefined operator!=(" + object->cls->name + ", " + other->cls->name + ")");
            });
            cls->methods["=="] = uva::lang::method("==", method_storage_type::instance_method, {"other"}, [interpreter, cls](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
                T& value = object->as<T>();
                std::shared_ptr<uva::lang::object> other = params[0];
                if(other->cls == interpreter->DoubleClass) {
                    return value == other->as<double>() ? std::make_shared<uva::lang::object>(interpreter->TrueClass) : std::make_shared<uva::lang::object>(interpreter->FalseClass);
                } else if(other->cls == interpreter->IntegerClass) {
                    return value == other->as<int>() ? std::make_shared<uva::lang::object>(interpreter->TrueClass) : std::make_shared<uva::lang::object>(interpreter->FalseClass);
                } else if(other->cls == interpreter->FloatClass) {
                    return value == other->as<float>() ? std::make_shared<uva::lang::object>(interpreter->TrueClass) : std::make_shared<uva::lang::object>(interpreter->FalseClass);
                }

                throw std::runtime_error("undefined operator==(" + object->cls->name + ", " + other->cls->name + ")");
            });
            cls->methods["<"] = uva::lang::method("<", method_storage_type::instance_method, {"other"}, [interpreter, cls](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
                T& value = object->as<T>();
                std::shared_ptr<uva::lang::object> other = params[0];
                if(other->cls == interpreter->DoubleClass) {
                    return value < other->as<double>() ? std::make_shared<uva::lang::object>(interpreter->TrueClass) : std::make_shared<uva::lang::object>(interpreter->FalseClass);
                } else if(other->cls == interpreter->IntegerClass) {
                    return value < other->as<int>() ? std::make_shared<uva::lang::object>(interpreter->TrueClass) : std::make_shared<uva::lang::object>(interpreter->FalseClass);
                } else if(other->cls == interpreter->FloatClass) {
                    return value < other->as<float>() ? std::make_shared<uva::lang::object>(interpreter->TrueClass) : std::make_shared<uva::lang::object>(interpreter->FalseClass);
                }

                throw std::runtime_error("undefined operator<(" + object->cls->name + ", " + other->cls->name + ")");
            });
            cls->methods[">"] = uva::lang::method(">", method_storage_type::instance_method, {"other"}, [interpreter, cls](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params) {
                T& value = object->as<T>();
                std::shared_ptr<uva::lang::object> other = params[0];
                if(other->cls == interpreter->DoubleClass) {
                    return value > other->as<double>() ? std::make_shared<uva::lang::object>(interpreter->TrueClass) : std::make_shared<uva::lang::object>(interpreter->FalseClass);
                } else if(other->cls == interpreter->IntegerClass) {
                    return value > other->as<int>() ? std::make_shared<uva::lang::object>(interpreter->TrueClass) : std::make_shared<uva::lang::object>(interpreter->FalseClass);
                } else if(other->cls == interpreter->FloatClass) {
                    return value > other->as<float>() ? std::make_shared<uva::lang::object>(interpreter->TrueClass) : std::make_shared<uva::lang::object>(interpreter->FalseClass);
                }

                throw std::runtime_error("undefined operator>(" + object->cls->name + ", " + other->cls->name + ")");
            });
        }
    };
};