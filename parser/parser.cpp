#include "parser.hpp"

#include <uva/file.hpp>
#include <console.hpp>

#include <exception>
#include <iostream>

#include <lang/object.hpp>
#include <lang/class.hpp>
#include <lang/method.hpp>

using namespace uva;
using namespace lang;

uva::lang::parser::parser()
{
}

uva::lang::parser::ast_node uva::lang::parser::parse_fn_call_params(uva::lang::lexer &lexer)
{
    ast_node params_node(ast_node_type::ast_node_fn_params);

    ast_node param_node = parse_node(lexer);

    while(param_node.token().content() != ")") {
        if(param_node.type() == uva::lang::parser::ast_node_type::ast_node_valuedecl || param_node.type() == uva::lang::parser::ast_node_type::ast_node_fn_call) {
            params_node.add_child(param_node);
        } else if(param_node.token().content() == ",") {
            // Do nothing
        } else {
            param_node.token().throw_error_at_current_position("Expected literal or identifier");
        }

        param_node = parse_node(lexer);
    }

    return params_node;
}

uva::lang::parser::ast_node parser::parse_node(uva::lang::lexer &lexer)
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

                ast_node value_node = parse_node(lexer);

                var_node.add_child(value_node);

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

                ast_node fn_context = parse_node(lexer);

                if(fn_context.type() != ast_node_type::ast_node_context) {
                    token.throw_error_at_current_position("Expected context after method declaration");
                }

                method_node.add_child(std::move(fn_context));

                return method_node;
            } else if(token.content() == "return") {
                ast_node return_node(std::move(token), ast_node_type::ast_node_fn_return);

                token = lexer.next_token();

                if(token.type() != lexer::token_type::token_literal && token.type() != lexer::token_type::token_identifier) {
                    token.throw_error_at_current_position("Expected literal or identifier after 'return'");
                }

                return_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_valuedecl)));

                return return_node;
            }
            else if(token.content() == "if") {
                ast_node if_node(ast_node_type::ast_node_conditional);
                if_node.add_child(ast_node(std::move(token), ast_node_type::ast_node_decltype));

                token = lexer.next_token();

                if(token.content() != "(") {
                    token.throw_error_at_current_position("Expected '(' after 'if'");
                }

                ast_node condition_node(ast_node_type::ast_node_condition);

                ast_node node = parse_node(lexer);

                condition_node.add_child(std::move(node));

                if_node.add_child(std::move(condition_node));

                ast_node if_child = parse_node(lexer);

                if(if_child.token().content() != ")") {
                    if_child.token().throw_error_at_current_position("Expected ')' after 'if' condition");
                }

                ast_node if_context = parse_node(lexer);

                if(if_context.type() != ast_node_type::ast_node_context) {
                    token.throw_error_at_current_position("Expected context after 'if'");
                }

                if_node.add_child(std::move(if_context));

                return if_node;
            } else if(token.content() == "foreach")  {
                ast_node foreach_node(ast_node_type::ast_node_foreach);
                foreach_node.add_child(ast_node(std::move(token), ast_node_type::ast_node_decltype));

                token = lexer.next_token();

                if(token.content() != "(") {
                    token.throw_error_at_current_position("Expected '(' after 'foreach'");
                }

                token = lexer.next_token();

                if(token.content() != "var") {
                    token.throw_error_at_current_position("Expected 'var' after '('");
                }

                ast_node var_node(ast_node_type::ast_node_vardecl);
                var_node.add_child(ast_node(std::move(token), ast_node_type::ast_node_decltype));

                token = lexer.next_token();

                if(token.type() != lexer::token_type::token_identifier) {
                    token.throw_error_at_current_position("Expected variable name after 'var'");
                }

                var_node.add_child(ast_node(std::move(token), ast_node_type::ast_node_declname));

                foreach_node.add_child(std::move(var_node));

                token = lexer.next_token();

                if(token.content() != "in") {
                    token.throw_error_at_current_position("Expected 'in' after variable name");
                }

                token = lexer.next_token();

                if(token.type() != lexer::token_type::token_identifier) {
                    token.throw_error_at_current_position("Expected identifier after 'in'");
                }

                foreach_node.add_child(ast_node(std::move(token), ast_node_type::ast_node_valuedecl));

                token = lexer.next_token();

                if(token.content() != ")") {
                    token.throw_error_at_current_position("Expected ')' after 'foreach' declaration");
                }

                ast_node context_node = parse_node(lexer);

                if(context_node.type() != ast_node_type::ast_node_context) {
                    token.throw_error_at_current_position("Expected context after 'foreach'");
                }

                foreach_node.add_child(std::move(context_node));

                return foreach_node;
            }
            else {
                token.throw_error_at_current_position("Unexpected keyword");
            }
            break;
        case lexer::token_type::token_delimiter:
            if(token.content() == ";") {
                // ; in the middle of the code is considered a whitespace
                return parse_node(lexer);
            } else if(token.content() == "{") {
                ast_node context_node(ast_node_type::ast_node_context);

                ast_node context_child = parse_node(lexer);

                while(context_child.token().content() != "}") {
                    context_node.add_child(std::move(context_child));
                    context_child = parse_node(lexer);
                }

                return context_node;
            }

            return ast_node(std::move(token), ast_node_type::ast_node_undefined);
            break;
        case lexer::token_type::token_literal:
            return ast_node(std::move(token), ast_node_type::ast_node_valuedecl);
        break;
        case lexer::token_type::token_identifier:
            // Can be: variable reference/assingment, Function call
            
            // First check if it has '(' after the identifier

            token = lexer.next_token();

            if(token.content() == "(") {
                // Go back to the identifier
                token = lexer.previous_token();

                // Function call
                ast_node method_node(ast_node_type::ast_node_fn_call);
                method_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_declname)));

                token = lexer.next_token(); // (
                
                ast_node params = parse_fn_call_params(lexer);

                method_node.add_child(std::move(params));

                // Can have fn1().fn2().fn3()...

                token = lexer.next_token();

                while(token.content() == ".") {
                    ast_node next_node = parse_node(lexer);

                    if(next_node.type() != ast_node_type::ast_node_fn_call) {
                        token.throw_error_at_current_position("Expected function call after '.'");
                    }

                    ast_node* next_node_object = next_node.child_from_type(ast_node_type::ast_node_fn_object);

                    ast_node object_node(ast_node_type::ast_node_fn_object);
                    object_node.add_child(std::move(method_node));

                    method_node = std::move(next_node);

                    if(next_node_object == nullptr) {
                        method_node.add_child(std::move(object_node));   
                    } else {
                        // Need to recursively add the method to the last fn_object

                        while(true) {
                            if(next_node_object->childrens().empty()) {
                                break;
                            }

                            next_node_object = next_node_object->childrens().data();

                            auto temp = next_node_object->child_from_type(ast_node_type::ast_node_fn_object);

                            if(temp == nullptr) {
                                break;
                            }

                            next_node_object = temp;
                        }

                        next_node_object->add_child(std::move(object_node));
                    }

                    token = lexer.next_token();
                }

                lexer.rollback_token();

                return method_node;
            }
            else if(token.content() == ".") {
                // Go back to the identifier
                uva::lang::lexer::token previous_token = lexer.previous_token();

                token = lexer.next_token(); // . again

                ast_node next_node = parse_node(lexer);

                if(next_node.type() != ast_node_type::ast_node_fn_call) {
                    token.throw_error_at_current_position("Expected function call after '.'");
                }
                
                ast_node object_node(ast_node_type::ast_node_fn_object);
                object_node.add_child(std::move(ast_node(std::move(previous_token), ast_node_type::ast_node_declname)));

                next_node.add_child(std::move(object_node));

                return next_node;
            }
            else if(token.content() == "!" || token.content() == "?") {
                // ! or ? after the identifier is a method call
                // Go back to the identifier
                uva::lang::lexer::token original_name_token = lexer.previous_token();

                token = lexer.next_token(); // ! again

                ast_node method_node(ast_node_type::ast_node_fn_call);

                uva::lang::lexer::token name_token(original_name_token.start, token.end, original_name_token.content() + token.content(), lexer::token_type::token_identifier);

                method_node.add_child(std::move(ast_node(std::move(name_token), ast_node_type::ast_node_declname)));

                // The parser separates the identifier from the '!' token.
                // In this case, the () is not required

                token = lexer.next_token();

                if(token.content() == "(") {
                    ast_node params_node = parse_fn_call_params(lexer);
                    method_node.add_child(std::move(params_node));
                } else {
                    lexer.rollback_token();
                }

                return method_node;
            } else if(token.type() == uva::lang::lexer::token_operator) {
                // An identifier + operator

                ast_node node(std::move(token), ast_node_type::ast_node_fn_call);
                node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_declname)));

                // Go back to the identifier
                token = lexer.previous_token();

                ast_node object_node(ast_node_type::ast_node_fn_object);
                object_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_declname)));

                node.add_child(std::move(object_node));

                token = lexer.next_token(); // operator

                ast_node right_node = parse_node(lexer);

                ast_node params_node = ast_node(ast_node_type::ast_node_fn_params);
                params_node.add_child(std::move(right_node));

                node.add_child(std::move(params_node));

                // Check if we are already in an operator

                token = lexer.next_token();

                while(token.type() == lexer::token_type::token_operator) {

                    lexer.rollback_token();

                    ast_node next_node = parse_node(lexer);

                    if(next_node.type() == ast_node_type::ast_node_fn_call) {
                        ast_node next_node_object(ast_node_type::ast_node_fn_object);
                        next_node_object.add_child(std::move(node));

                        next_node.add_child(std::move(next_node_object));

                        return next_node;
                    } else {
                        token.throw_error_at_current_position("Expected function call after operator");
                    }
                }

                lexer.rollback_token();

                return node;
            }
            else {
                // Go back to the identifier
                token = lexer.previous_token();

                // Variable reference/assignment
                // Only variable reference is implemented for now
                ast_node node(std::move(token), ast_node_type::ast_node_valuedecl);
                return node;
            }
            break;
        case lexer::token_type::token_operator: {
            if(token.content() == "[") {
                // Array declaration
                ast_node array_node(ast_node_type::ast_node_arraydecl);

                while(true) {
                    token = lexer.next_token();

                    if(token.type() != lexer::token_type::token_literal && token.type() != lexer::token_type::token_identifier) {
                        token.throw_error_at_current_position("Expected literal or identifier in array declaration");
                    }

                    array_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_valuedecl)));

                    token = lexer.next_token();

                    if(token.content() == ",") {
                        continue;
                    } else if(token.content() == "]") {
                        break;
                    } else {
                        token.throw_error_at_current_position("Expected ',' or ']'");
                    }
                }

                return array_node;
            } else {
                ast_node operator_node(std::move(token), ast_node_type::ast_node_fn_call);
                operator_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_declname)));

                ast_node right_node = parse_node(lexer);
                
                ast_node params_node = ast_node(ast_node_type::ast_node_fn_params);
                params_node.add_child(std::move(right_node));

                operator_node.add_child(std::move(params_node));

                return operator_node;
            }
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