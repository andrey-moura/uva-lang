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
                ast_node_context,
                ast_node_classdecl,

                ast_node_fn_decl,
                ast_node_fn_return,
                ast_node_fn_call,

                ast_node_valuedecl,
                ast_node_vardecl,

                ast_node_decltype,
                ast_node_declname,

                ast_node_conditional,
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

                std::string_view child_content_from_type(const ast_node_type& __type) const {
                    return child_token_from_type(__type)->content();
                }

                std::string_view decname() const {
                    return child_content_from_type(ast_node_type::ast_node_declname);
                }

                std::string_view value() const {
                    return child_content_from_type(ast_node_type::ast_node_valuedecl);
                }

                const ast_node* condition() const {
                    return child_from_type(ast_node_type::ast_node_condition);
                }

                const ast_node* block() const {
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
        };
    }
}; // namespace uva