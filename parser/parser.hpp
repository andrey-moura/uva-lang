#pragma once

#include <filesystem>
#include <map>
#include <functional>
#include <string>

#include <uva/core.hpp>

#include <lexer/lexer.hpp>

namespace uva
{
    namespace lang
    {
        class parser
        {
        public:
            parser();
            ~parser() = default;
        public:
            enum ast_node_type {
                ast_node_undefined,

                ast_node_unit,
                ast_node_expansion,

                ast_node_context,

                ast_node_classdecl,
                ast_node_classdecl_base,

                ast_node_fn_decl,
                ast_node_fn_return,
                ast_node_fn_call,
                ast_node_fn_params,
                ast_node_fn_object,

                ast_node_valuedecl,
                ast_node_arraydecl,
                ast_node_dictionarydecl,
                ast_node_vardecl,
                ast_node_foreach,

                ast_node_decltype,
                ast_node_declname,

                ast_node_conditional,
                ast_node_else,
                ast_node_condition
            };
            class ast_node
            {
            public:
                ast_node()
                    : m_type(ast_node_type::ast_node_undefined) {
                    
                }
                ast_node(ast_node_type __type)
                    : m_type(__type) {
                    
                }
                ast_node(uva::lang::lexer::token __token, ast_node_type __type)
                    : m_token(__token), m_type(__type) {
                    
                }
            protected:
                uva::lang::lexer::token m_token;
                ast_node_type m_type;
                std::vector<ast_node> m_children;
            public:
                bool is_undefined() const {
                    return m_type == ast_node_type::ast_node_undefined;
                }
            // Setters
            public:
                void add_child(ast_node child) {
                    m_children.push_back(std::move(child));
                }
                void set_type(ast_node_type __type)
                {
                    m_type = __type;
                }
                void set_token(uva::lang::lexer::token __token)
                {
                    m_token = std::move(__token);
                }
            // Getters
            public:
                const uva::lang::lexer::token& token() const {
                    return m_token;
                }

                std::vector<ast_node>& childrens() {
                    return m_children;
                }

                const std::vector<ast_node>& childrens() const {
                    return m_children;
                }

                ast_node_type type() const {
                    return m_type;
                }

                const ast_node* child_from_type(const uva::lang::parser::ast_node_type& __type) const {
                    for(auto& child : m_children) {
                        if(child.type() == __type) {
                            return &child;
                        }
                    }

                    return nullptr;
                }

                ast_node* child_from_type(const uva::lang::parser::ast_node_type& __type) {
                    for(auto& child : m_children) {
                        if(child.type() == __type) {
                            return &child;
                        }
                    }

                    return nullptr;
                }

                const uva::lang::lexer::token* child_token_from_type(const uva::lang::parser::ast_node_type& __type) const {
                    return &child_from_type(__type)->token();
                }

                const std::string& child_content_from_type(const ast_node_type& __type) const {
                    return child_token_from_type(__type)->content();
                }

                const std::string& decname() const {
                    return child_content_from_type(ast_node_type::ast_node_declname);
                }

                const std::string& value() const {
                    return child_content_from_type(ast_node_type::ast_node_valuedecl);
                }

                const ast_node* condition() const {
                    return child_from_type(ast_node_type::ast_node_condition);
                }

                const ast_node* block() const {
                    return child_from_type(ast_node_type::ast_node_context);
                }

                const ast_node* context() const {
                    return child_from_type(ast_node_type::ast_node_context);
                }
            };
        protected:
            std::filesystem::path current_path;
        public:
            std::filesystem::path absolute(const std::string& path) {
                return current_path / path;
            }
        public:
            uva::lang::parser::ast_node parse_node(uva::lang::lexer& lexer);
            uva::lang::parser::ast_node parse_all(uva::lang::lexer& lexer);

        // Commons extract functions used by parsers
        protected:
            /// @brief Extract a function call parameters. You must consume the ')' token.
            /// @param lexer The lexer.
            /// @return A function call parameters node.
            uva::lang::parser::ast_node extract_fn_call_params(uva::lang::lexer& lexer);
            /// @brief Extract a function call.
            /// @param lexer The lexer.
            /// @return A function call node.
            uva::lang::parser::ast_node extract_fn_call(uva::lang::lexer& lexer);

            uva::lang::parser::ast_node parse_identifier_or_literal(uva::lang::lexer& lexer, bool chain = true);
        // Parsers functions
        protected:
            /// @brief Parse a keyword.
            /// @param lexer The lexer.
            /// @return A keyword node.
            uva::lang::parser::ast_node parse_keyword(uva::lang::lexer& lexer);
            /// @brief Parse a delimiter.
            /// @param lexer The lexer.
            /// @return A delimiter node.
            uva::lang::parser::ast_node parse_delimiter(uva::lang::lexer& lexer);
            /// @brief Parse a context.
            /// @param lexer The lexer.
            /// @return A context node.
            uva::lang::parser::ast_node parse_eof(uva::lang::lexer& lexer);
            /// @brief Parse a preprocessing directive.
            /// @param lexer The lexer.
            /// @return An exception
            uva::lang::parser::ast_node parse_preprocessor(uva::lang::lexer& lexer);
        protected:
            uva::lang::parser::ast_node parse_keyword_class(uva::lang::lexer& lexer);
            uva::lang::parser::ast_node parse_keyword_var(uva::lang::lexer& lexer);
            uva::lang::parser::ast_node parse_keyword_function(uva::lang::lexer& lexer);
            uva::lang::parser::ast_node parse_keyword_return(uva::lang::lexer& lexer);
            uva::lang::parser::ast_node parse_keyword_if(uva::lang::lexer& lexer);
            uva::lang::parser::ast_node parse_keyword_foreach(uva::lang::lexer& lexer);
        };
    }
}; // namespace uva