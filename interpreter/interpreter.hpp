#pragma once

#include <vector>
#include <memory>

#include <uva/var.hpp>
#include <parser/parser.hpp>
#include <lang/class.hpp>
#include <lang/method.hpp>
#include <lang/object.hpp>

namespace uva
{
    namespace lang
    {
        // The context of the interpreter execution. It is relative to a block.
        struct interpreter_context
        {
            std::shared_ptr<uva::lang::structure> cls;
            std::shared_ptr<uva::lang::object> self;
            std::map<std::string, std::shared_ptr<uva::lang::object>> variables;

            bool has_returned = false;
            std::shared_ptr<uva::lang::object> return_value;
        };
        // This class is responsible of storing all resources needed by an uvalang program.
        // It will store all classes, objects, methods, variables, call stack, etc.
        class interpreter
        {
        public:
            /// @brief Construct a new interpreter object. When the interpreter object is constructed, it will
            // initialize all resources needed to run the interpreter. If you want to declare the interpreter
            // but not initialize it, you can use a interpreter pointer and initialize it later.
            interpreter();
            ~interpreter() = default;
        public:
            /// @brief Load a class into the vm. The class is kept alive by the vm untill it is destroyed.
            /// @param cls The class to be loaded. It is kept alive by the vm untill it is destroyed. It is globally accessible.
            void load(std::shared_ptr<uva::lang::structure> cls);

            /// @brief Exeuctes a syntax tree into the interpreter. Note that if the code has while loops with no exit condition, this method will never return.
            /// @param cls The syntax tree to exeuctes. All its childs (not recursively) will be executed.
            std::shared_ptr<uva::lang::object> execute(uva::lang::parser::ast_node source_code, std::shared_ptr<uva::lang::object> object = nullptr);

            std::shared_ptr<uva::lang::object> execute_all(std::vector<uva::lang::parser::ast_node>::const_iterator begin, std::vector<uva::lang::parser::ast_node>::const_iterator end, std::shared_ptr<uva::lang::object> object = nullptr);
            std::shared_ptr<uva::lang::object> execute_all(uva::lang::parser::ast_node source_code, std::shared_ptr<uva::lang::object> object = nullptr);

            /// @brief The global false class.
            std::shared_ptr<uva::lang::structure> FalseClass;
            /// @brief The global true class.
            std::shared_ptr<uva::lang::structure> TrueClass;

            /// @brief The global std class.
            std::shared_ptr<uva::lang::structure> StdClass;

            /// @brief The global string class.
            std::shared_ptr<uva::lang::structure> StringClass;

            /// @brief The global integer class.
            std::shared_ptr<uva::lang::structure> IntegerClass;

            /// @brief The global file class.
            std::shared_ptr<uva::lang::structure> FileClass;

            /// @brief The global array class.
            std::shared_ptr<uva::lang::structure> ArrayClass;

            template<typename T>
            std::shared_ptr<uva::lang::object> instantiate(std::shared_ptr<uva::lang::structure> cls, T value)
            {
                auto obj = std::make_shared<uva::lang::object>(cls);
                obj->native = new T(std::move(value));

                return obj;
            }

            std::shared_ptr<uva::lang::object> call(std::shared_ptr<uva::lang::structure> cls, std::shared_ptr<uva::lang::object> object, const uva::lang::method& method, std::vector<std::shared_ptr<uva::lang::object>> params);

            std::shared_ptr<uva::lang::structure> find_class(const std::string_view& name) {
                for(auto& cls : classes) {
                    if(cls->name == name) {
                        return cls;
                    }
                }

                return nullptr;
            }

            const std::shared_ptr<uva::lang::object> node_to_object(const uva::lang::parser::ast_node& node);
        protected:
            /// @brief The global context stack.
            interpreter_context global_context;

            /// @brief The current context.
            interpreter_context current_context;

            /// @brief The call stack.
            std::vector<interpreter_context> stack;

            void push_context() { stack.push_back(current_context); }
            void pop_context() { 
                if(stack.empty()) {
                    throw std::runtime_error("interpreter: unexpected end of file");
                }

                current_context = stack.back();
                stack.pop_back();
            }
        protected:
            /// @brief Initialize the interpreter. This method will create the global classes and objects. It also load extensions.
            void init();
        private:
            std::vector<std::shared_ptr<uva::lang::structure>> classes;
        };
    }  
}; // namespace uva