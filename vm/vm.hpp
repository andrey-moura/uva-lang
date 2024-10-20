#pragma once

#include <vector>
#include <memory>

#include <uva/var.hpp>
#include <parser/class.hpp>
#include <parser/method.hpp>
#include <parser/object.hpp>

namespace uva
{
    namespace lang
    {
        // The context of the VM execution. It is relative to a block.
        struct vm_context
        {
            std::shared_ptr<uva::lang::structure> cls;
            std::shared_ptr<uva::lang::object> self;
            std::map<std::string, std::shared_ptr<uva::lang::object>> variables;
            std::shared_ptr<vm_context> parent;
        };
        // This class is responsible of storing all resources needed by an uvalang program.
        // It will store all classes, objects, methods, variables, call stack, etc.
        class vm
        {
        public:
            /// @brief Construct a new vm object. When the vm object is constructed, it will
            // initialize all resources needed to run the vm. If you want to declare the VM
            // but not initialize it, you can use a vm pointer and initialize it later.
            vm();
            ~vm() = default;
        public:
            /// @brief Load a class into the vm. The class is kept alive by the vm untill it is destroyed.
            /// @param cls The class to be loaded. It is kept alive by the vm untill it is destroyed. It is globally accessible.
            void load(std::shared_ptr<uva::lang::structure> cls);

            /// @brief The global false class.
            std::shared_ptr<uva::lang::structure> False;
            /// @brief The global true class.
            std::shared_ptr<uva::lang::structure> True;

            /// @brief The global std class.
            std::shared_ptr<uva::lang::structure> Std;

            /// @brief Create an instance of @cls with the name @name at the global context.
            /// @param cls The class to instantiate.
            /// @param name The name of the instancieted object.
            /// @return The instancieted object
            std::shared_ptr<uva::lang::object> instantiate(std::shared_ptr<uva::lang::structure> cls, const std::string& name);

            std::shared_ptr<uva::lang::object> call(std::shared_ptr<uva::lang::object> object, const Method& method, const var& params = var());

            std::shared_ptr<uva::lang::structure> find_class(const std::string_view& name) {
                for(auto& cls : classes) {
                    if(cls->name == name) {
                        return cls;
                    }
                }

                return nullptr;
            }
        protected:
            /// @brief The global context stack.
            vm_context global_context;
        protected:
            /// @brief Initialize the vm. This method will create the global classes and objects. It also load extensions.
            void init();
        private:
            std::vector<std::shared_ptr<uva::lang::structure>> classes;
        };
    }  
}; // namespace uva