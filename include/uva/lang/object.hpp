#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <uva/var.hpp>

namespace uva
{
    namespace lang {
        class object;
        class structure;
        class interpreter;
        constexpr size_t MAX_NATIVE_SIZE = 32;
        class object : public std::enable_shared_from_this<object>
        {
        public:
            object(std::shared_ptr<uva::lang::structure> c);
            ~object();
        public:
            std::shared_ptr<uva::lang::structure> cls;
            std::shared_ptr<object> base_instance = nullptr;
            std::shared_ptr<object> derived_instance = nullptr;

            std::map<std::string, std::shared_ptr<uva::lang::object>> instance_variables;
            // #ifdef __UVA_DEBUG__
            // uva::lang::object* debug_object = this;

            // __attribute__((noinline)) __attribute__((used)) std::string debug_string()
            // {
            //     return to_var().to_s();
            // }
            // #endif
        protected:
            // A pointer to the native object
            void* native_ptr = nullptr;
            // The native object
            uint8_t native[MAX_NATIVE_SIZE] = {0};
            // The object destructor ptr.
            void (*native_destructor)(object* obj) = nullptr;
            
            void initialize(uva::lang::interpreter* interpreter);
        public:
            /// @brief Initialize the object with a value.
            /// @tparam T The type of the value.
            /// @param cls The class of the object.
            /// @param value The pointer to the value. This will be deleted when the object is destroyed.
            /// @return Returns a shared pointer to the object.
            template<typename T>
            static std::shared_ptr<uva::lang::object> instantiate(uva::lang::interpreter* interpreter, std::shared_ptr<uva::lang::structure> cls, T* value)
            {
                auto obj = std::make_shared<uva::lang::object>(cls);
                obj->set_native_ptr<T>(obj.get(), value);

                obj->initialize(interpreter);

                return obj;
            }

            /// @brief Initialize the object with a value.
            /// @tparam T The type of the value.
            /// @param cls The class of the object.
            /// @param value The value.
            /// @return Returns a shared pointer to the object.
            template<typename T>
            static std::enable_if<!std::is_pointer<T>::value, std::shared_ptr<uva::lang::object>>::type instantiate(uva::lang::interpreter* interpreter, std::shared_ptr<uva::lang::structure> cls, T value)
            {
                auto obj = std::make_shared<uva::lang::object>(cls);

                obj->set_native<T>(std::move(value));

                obj->initialize(interpreter);

                return obj;
            }

            template<typename T>
            void set_native(T value) {
                if(native_destructor) {
                    native_destructor(this);
                }

                bool should_destroy = false;

                if constexpr(sizeof(T) <= MAX_NATIVE_SIZE) {
                    if constexpr(std::is_arithmetic<T>::value) {
                        // Boolean, integer, float, etc.
                        *((T*)(&this->native)) = value;
                    } else {
                        new ((T*)(&this->native)) T(std::move(value));
                        should_destroy = true;
                    }
                } else {
                    this->native_ptr = new T(std::move(value));
                    should_destroy = true;
                }

                set_destructor<T>(this);
            }

            template<typename T>
            void set_native_ptr(T* ptr) {
                this->native_ptr = (void*)ptr;
                set_destructor<T>(this);
            }

            template<typename T>
            T* move_native_ptr() {
                T* ptr = (T*)this->native_ptr;
                this->native_ptr = nullptr;
                this->native_destructor = nullptr;
                return ptr;
            }
        private:
            template <typename T>
            static void set_destructor(object* obj) {
                obj->native_destructor = [](object* obj) {
                    obj->log_native_destructor();

                    if(obj->native_ptr) {
                        delete (T*)obj->native_ptr;
                    } else {
                        if constexpr(std::is_arithmetic<T>::value) {
                            // Do nothing
                        } else {
                            ((T*)(&obj->native))->~T();
                        }
                    }
                };
            }

            void log_native_destructor();
        public:
            bool is_present() const;

            template<typename T>
            const T& as() const {
                if(native_ptr) {
                    return *static_cast<T*>(native_ptr);
                }

                return *static_cast<T*>((void*)native);
            }
            template<typename T>
            T& as() {
                if(native_ptr) {
                    return *static_cast<T*>(native_ptr);
                }

                return *static_cast<T*>((void*)native);
            }

            var to_var() const;
        };
    };
};