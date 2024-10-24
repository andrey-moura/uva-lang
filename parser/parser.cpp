#include "parser.hpp"

#include <uva/file.hpp>

#include <exception>
#include <iostream>

#include <parser/object.hpp>
#include <parser/class.hpp>
#include <parser/method.hpp>

using namespace uva;
using namespace lang;

uva::lang::parser::parser()
{
}

uva::lang::parser::ast_node parser::parse_node(uva::lang::lexer& lexer)
{
    uva::lang::lexer::token token = lexer.next_token();

    switch(token.type()) {
        case lexer::token_type::token_comment:
            // Do nothing
            // The comments is not discarted by the lexer because it is useful for debugging/intellisense

            return parse_node(lexer);
            break;
        case lexer::token_type::token_keyword:
            if(token.content() == "class") {
                ast_node class_node(ast_node_type::ast_node_classdecl);

                class_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_decltype)));

                token = lexer.next_token();

                if(token.type() != lexer::token_type::token_identifier) {
                    token.throw_error_at_current_position("Expected class name after 'class'");
                }

                class_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_declname)));

                token = lexer.next_token();

                if(token.content() != "{") {
                    token.throw_error_at_current_position("Expected '{' after class name");
                }

                ast_node class_child = parse_node(lexer);

                while(!class_child.is_undefined()) {
                    class_node.add_child(std::move(class_child));
                    class_child = parse_node(lexer);
                }

                return class_node;
            } else if(token.content() == "var") {
                ast_node var_node(std::move(token), ast_node_type::ast_node_vardecl);

                token = lexer.next_token();

                if(token.type() != lexer::token_type::token_identifier) {
                    token.throw_error_at_current_position("Expected variable name after 'var'");
                }

                var_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_declname)));

                token = lexer.next_token();

                if(token.content() != "=") {
                    token.throw_error_at_current_position("Expected '=' after variable name");
                }

                token = lexer.next_token();

                if(token.type() != lexer::token_type::token_literal) {
                    token.throw_error_at_current_position("Expected literal after '='");
                }

                var_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_valuedecl)));

                token = lexer.next_token();

                return var_node;
            } else if(token.content() == "function") {
                ast_node method_node(ast_node_type::ast_node_fn_decl);
                method_node.add_child(ast_node(std::move(token), ast_node_type::ast_node_decltype));

                token = lexer.next_token();

                if(token.type() != lexer::token_type::token_identifier) {
                    token.throw_error_at_current_position("Expected method name after 'function'");
                }

                method_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_declname)));

                token = lexer.next_token();

                if(token.content() != "(") {
                    token.throw_error_at_current_position("Expected '(' after method name");
                }

                token = lexer.next_token();

                while(token.content() != ")") {
                    if(token.type() != lexer::token_type::token_identifier) {
                        token.throw_error_at_current_position("Expected parameter name");
                    }

                    method_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_declname)));

                    token = lexer.next_token();

                    if(token.content() == ",") {
                        token = lexer.next_token();
                    } else if(token.content() != ")") {
                        token.throw_error_at_current_position("Expected ',' or ')'");
                    }
                }

                token = lexer.next_token();

                if(token.content() != "{") {
                    token.throw_error_at_current_position("Expected '{' after method declaration");
                }

                ast_node method_child = parse_node(lexer);

                while(method_child.token().content() != "}") {
                    method_node.add_child(std::move(method_child));
                    method_child = parse_node(lexer);
                }

                return method_node;
            } else if(token.content() == "return") {
                ast_node return_node(std::move(token), ast_node_type::ast_node_fn_return);

                token = lexer.next_token();

                if(token.type() != lexer::token_type::token_literal) {
                    token.throw_error_at_current_position("Expected literal after 'return'");
                }

                return_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_valuedecl)));

                return return_node;
            } else {
                token.throw_error_at_current_position("Unexpected keyword");
            }
            break;
        case lexer::token_type::token_delimiter:
            if(token.content() == ";") {
                // ; in the middle of the code is considered a whitespace
                return parse_node(lexer);
            }

            return ast_node(std::move(token), ast_node_type::ast_node_undefined);
            break;
        case lexer::token_type::token_identifier:
            // Can be: variable assingment, method call
            
            // First check if it has '(' after the identifier

            token = lexer.next_token();

            if(token.content() == "(") {
                // Go back to the identifier
                token = lexer.previous_token();

                // Method call
                ast_node method_node(ast_node_type::ast_node_fn_call);

                method_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_declname)));

                token = lexer.next_token(); // ( again
                token = lexer.next_token();

                std::string_view content;

                while((content = token.content()) != ")") {
                    if(token.type() != lexer::token_type::token_literal && token.type() != lexer::token_type::token_identifier) {
                        token.throw_error_at_current_position("Expected literal or identifier in function call");
                    }

                    method_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_valuedecl)));

                    token = lexer.next_token();

                    if(token.content() == ",") {
                        token = lexer.next_token();
                    } else if(token.content() != ")") {
                        token.throw_error_at_current_position("Expected ',' or ')'");
                    }
                }

                return method_node;
            } else {
                token.throw_error_at_current_position("Unexpected identifier");
            }
            break;
        default:
            token.throw_error_at_current_position("Unexpected token");
            break;
    }

    // This should never happen
    // This is just to avoid the compiler warning
    throw std::runtime_error("parser: unknown node");
}

uva::lang::parser::ast_node uva::lang::parser::parse_all(uva::lang::lexer &lexer)
{
    ast_node root_node(ast_node_type::ast_node_unit);

    // TODO: Parse other nodes

    ast_node child = parse_node(lexer);
    root_node.add_child(std::move(child));
    return root_node;
}

uva::lang::object::~object()
{
    uva::console::log_debug("{}#{} destroyed", cls->name, (void*)this);
}

uva::lang::structure::~structure()
{
    uva::console::log_debug("{}#Class destroyed", name);
}


uva::lang::structure::structure(const std::string& __name, std::vector<uva::lang::method> __methods)
    : name(__name)
{
    uva::console::log_debug("{}#Class created", name);
}
