#pragma once

#include <vector>
#include <memory>

#include <uva/var.hpp>
#include <uva/lang/parser.hpp>
#include <uva/lang/class.hpp>
#include <uva/lang/method.hpp>
#include <uva/lang/object.hpp>

namespace uva
{
    namespace lang
    {
        class extension;
        // The context of the interpreter execution. It is relative to a block.
        struct interpreter_context
        {
            std::shared_ptr<uva::lang::structure> cls;
            std::shared_ptr<uva::lang::object> self;
            std::map<std::string, std::shared_ptr<uva::lang::object>> variables;
            std::map<std::string, uva::lang::method> functions;

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
            std::shared_ptr<uva::lang::object> execute(uva::lang::parser::ast_node source_code, std::shared_ptr<uva::lang::object>& object);

            std::shared_ptr<uva::lang::object> execute_all(std::vector<uva::lang::parser::ast_node>::const_iterator begin, std::vector<uva::lang::parser::ast_node>::const_iterator end, std::shared_ptr<uva::lang::object>& object);
            std::shared_ptr<uva::lang::object> execute_all(uva::lang::parser::ast_node source_code, std::shared_ptr<uva::lang::object>& object);

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

            /// @brief The global double class.
            std::shared_ptr<uva::lang::structure> DoubleClass;

            /// @brief The global float class.
            std::shared_ptr<uva::lang::structure> FloatClass;

            /// @brief The global file class.
            std::shared_ptr<uva::lang::structure> FileClass;

            /// @brief The global array class.
            std::shared_ptr<uva::lang::structure> ArrayClass;

            /// @brief The global null class.
            std::shared_ptr<uva::lang::structure> NullClass;

            /// @brief The global dictionary class.
            std::shared_ptr<uva::lang::structure> DictionaryClass;

            std::shared_ptr<uva::lang::object> call(
                std::shared_ptr<uva::lang::structure> cls,
                std::shared_ptr<uva::lang::object>    object,
                const uva::lang::method&              method,
                std::vector<std::shared_ptr<uva::lang::object>> positional_params,
                std::map<std::string, std::shared_ptr<uva::lang::object>> named_params = {}
            );

            std::shared_ptr<uva::lang::structure> find_class(const std::string_view& name) {
                for(auto& cls : classes) {
                    if(cls->name == name) {
                        return cls;
                    }
                }

                return nullptr;
            }

            const std::shared_ptr<uva::lang::object> node_to_object(const uva::lang::parser::ast_node& node, std::shared_ptr<uva::lang::structure> cls = nullptr, std::shared_ptr<uva::lang::object> object = nullptr);
            std::shared_ptr<uva::lang::object> var_to_object(var v);

            void load_extension(uva::lang::extension* extension);

            void start_extensions();
        protected:
            /// @brief The global context stack.
            interpreter_context global_context;

            /// @brief The current context.
            interpreter_context current_context;

            /// @brief The call stack.
            std::vector<interpreter_context> stack;

            std::vector<uva::lang::extension*> extensions;

            void push_context(bool inherit = false) {
                stack.push_back(std::move(current_context));
                current_context = interpreter_context();

                if(inherit) {
                    current_context.variables = stack.back().variables;
                    current_context.functions = stack.back().functions;
                }
            }
            void pop_context() { 
                if(stack.empty()) {
                    throw std::runtime_error("interpreter: unexpected end of file");
                }

                current_context = std::move(stack.back());
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