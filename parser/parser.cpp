#include "parser.hpp"

#include <uva/file.hpp>
#include <console.hpp>

#include <exception>
#include <iostream>
#include <regex>

#include <lang/object.hpp>
#include <lang/class.hpp>
#include <lang/method.hpp>

using namespace uva;
using namespace lang;

// Keep the order as uva::lang::lexer::token_type
uva::lang::parser::parser_function uva::lang::parser::s_parsers[uva::lang::lexer::token_type::token_type_max] = {
    nullptr,
    &uva::lang::parser::parse_comment,
    &uva::lang::parser::parse_keyword,
    &uva::lang::parser::parse_identifier,
    &uva::lang::parser::parse_literal,
    &uva::lang::parser::parse_delimiter,
    &uva::lang::parser::parse_operator,
    &uva::lang::parser::parse_eof,
};

// TODO: move to uva::file

std::string wildcard_to_regex(const std::string& wildcard) {
    std::string regex_pattern = "^";
    for (char ch : wildcard) {
        switch (ch) {
            case '*':
                regex_pattern += ".*"; // '*' corresponde a qualquer sequência de caracteres
                break;
            case '?':
                regex_pattern += ".";  // '?' corresponde a um único caractere
                break;
            case '.':
                regex_pattern += "\\."; // Escape do ponto, pois em regex, '.' é um caractere especial
                break;
            default:
                regex_pattern += ch;    // Adiciona o caractere literal
                break;
        }
    }
    regex_pattern += "$"; // Final da expressão regular
    return regex_pattern;
}

// Função para listar arquivos com base em um wildcard
std::vector<std::string> list_files_with_wildcard(const std::filesystem::path& base_path, std::string pattern) {
    std::vector<std::string> files;
    pattern = "*/" + pattern; // Adiciona um coringa para buscar em subdiretórios
    std::regex regex_pattern(wildcard_to_regex(pattern));  // Converte o padrão para regex

    for (const auto& entry : std::filesystem::recursive_directory_iterator(base_path)) {
        if (std::filesystem::is_regular_file(entry.path())) {
            std::string filename = entry.path().string();

            // Verifica se o arquivo corresponde ao padrão
            if (std::regex_match(filename, regex_pattern)) {
                files.push_back(filename);
            }
        }
    }

    return files;
}

uva::lang::parser::parser()
{
}

uva::lang::parser::ast_node parser::parse_node(uva::lang::lexer &lexer)
{
    const uva::lang::lexer::token& token = lexer.see_next();

    uva::lang::lexer::token_type token_type = token.type();

    if((int)token_type >= uva::lang::lexer::token_type::token_type_max) {
        token.throw_error_at_current_position("Unexpected token");
    }

    parser_function parser = s_parsers[(int)token_type];

    if(parser == nullptr) {
        token.throw_error_at_current_position("Unexpected token");
    }

    return (this->*parser)(lexer);
}

uva::lang::parser::ast_node uva::lang::parser::parse_all(uva::lang::lexer &lexer)
{
    ast_node root_node(ast_node_type::ast_node_unit);

    do {
        ast_node child = parse_node(lexer);

        if(child.type() == ast_node_type::ast_node_expansion) {
            for(auto& expansion_child : child.childrens()) {
                for(auto& expansion_child_child : expansion_child.childrens()) {
                    if(expansion_child_child.token().type() != lexer::token_type::token_eof) {
                        root_node.add_child(std::move(expansion_child_child));
                    }
                }
            }
        } else {
            root_node.add_child(std::move(child));
        }
    } while(lexer.has_next_token());

    return root_node;
}

uva::lang::lexer::token uva::lang::parser::extract_identifier(uva::lang::lexer &lexer)
{
    uva::lang::lexer::token token      = lexer.next_token();
    uva::lang::lexer::token identifier = token;

    while(true) {
        token = lexer.next_token();

        if(token.content() == "." || token.content() == "->" || token.content() == "::") {
            identifier.merge(token);
            
            token = lexer.next_token();

            if(token.type() != lexer::token_type::token_identifier) {
                token.throw_error_at_current_position("Expected identifier after '.'");
            }

            identifier.merge(token);
        } else {
            lexer.rollback_token();
            break;
        }

    }

    return identifier;
}

uva::lang::parser::ast_node uva::lang::parser::extract_fn_call(uva::lang::lexer &lexer)
{
    // Function call
    uva::lang::lexer::token token = lexer.next_token(); 

    ast_node method_node(ast_node_type::ast_node_fn_call);
    method_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_declname)));

    token = lexer.next_token(); // (
    
    ast_node params = extract_fn_call_params(lexer);

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

uva::lang::parser::ast_node uva::lang::parser::extract_value(uva::lang::lexer& lexer)
{
    uva::lang::lexer::token token = lexer.see_next();

    switch (token.type())
    {
    case uva::lang::lexer::token_type::token_literal:
        // Eaisiest case, just return the value
        return uva::lang::parser::ast_node(lexer.next_token(), uva::lang::parser::ast_node_type::ast_node_valuedecl);
        break;
    case uva::lang::lexer::token_type::token_identifier:
        // Can be a function call or a member access
        return parse_identifier(lexer);
        break;
    case uva::lang::lexer::token_operator: {
        // Can be array []

        if(token.content() == "[") {
            ast_node array_node(ast_node_type::ast_node_arraydecl);

            // The token was seen, so we need to consume it
            token = lexer.next_token();

            while(token.content() != "]") {
                ast_node value_node = extract_value(lexer);

                if(value_node.type() != ast_node_type::ast_node_valuedecl) {
                    token.throw_error_at_current_position("Expected value in array");
                }

                array_node.add_child(std::move(value_node));

                token = lexer.next_token();

                if(token.content() != "," && token.content() != "]") {
                    token.throw_error_at_current_position("Expected ',' or ']'");
                }
            }

            return array_node;
        } else {
            token.throw_error_at_current_position("Unexpected operator");
        }
    }
    case uva::lang::lexer::token_type::token_delimiter: {
        // Can be a map {}

        if(token.content() == "{") {
            ast_node map_node(ast_node_type::ast_node_dictionarydecl);

            // The token was seen, so we need to consume it
            token = lexer.next_token();

            while(token.content() != "}") {
                ast_node key_node = extract_value(lexer);

                if(key_node.type() != ast_node_type::ast_node_valuedecl) {
                    token.throw_error_at_current_position("Expected key in map");
                }

                token = lexer.next_token();

                if(token.content() != ":") {
                    token.throw_error_at_current_position("Expected ':' after key in map");
                }

                ast_node value_node = extract_value(lexer);

                if(value_node.type() != ast_node_type::ast_node_valuedecl) {
                    token.throw_error_at_current_position("Expected value in map");
                }

                ast_node pair_node = ast_node(ast_node_type::ast_node_valuedecl);

                key_node.set_type(ast_node_type::ast_node_declname);
                value_node.set_type(ast_node_type::ast_node_valuedecl);

                pair_node.add_child(std::move(key_node));
                pair_node.add_child(std::move(value_node));

                map_node.add_child(std::move(pair_node));

                token = lexer.next_token();

                if(token.content() != "," && token.content() != "}") {
                    token.throw_error_at_current_position("Expected ',' or '}'");
                }
            }

            return map_node;
        } 
    }
    break;
    case uva::lang::lexer::token_type::token_keyword:
        if(token.content() == "new") {
           return parse_node(lexer);
        } else {
            token.throw_error_at_current_position("Unexpected keyword");
        }
        break;
    default:
        token.throw_error_at_current_position("Expected literal or identifier");
        break;
    }

    return ast_node();
}

uva::lang::parser::ast_node uva::lang::parser::extract_fn_call_params(uva::lang::lexer &lexer)
{
    ast_node params_node(ast_node_type::ast_node_fn_params);

    uva::lang::lexer::token token = lexer.see_next();

    while(token.content() != ")") {
        ast_node param_node = extract_value(lexer);
        params_node.add_child(std::move(param_node));

        token = lexer.see_next();
    }

    // The ')' token was seen, so we need to consume it
    lexer.next_token();

    return params_node;
}

uva::lang::parser::ast_node uva::lang::parser::parse_comment(uva::lang::lexer &lexer)
{
    lexer.next_token(); // Consume the comment token
    return parse_node(lexer);
}

uva::lang::parser::ast_node uva::lang::parser::parse_keyword(uva::lang::lexer &lexer)
{
    uva::lang::lexer::token token = lexer.next_token();

    if(token.content() == "class") {
        ast_node class_node(ast_node_type::ast_node_classdecl);

        class_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_decltype)));

        token = lexer.next_token();

        if(token.type() != lexer::token_type::token_identifier) {
            token.throw_error_at_current_position("Expected class name after 'class'");
        }

        class_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_declname)));

        token = lexer.next_token();

        if(token.content() == "extends" || token.content() == ":" || token.content() == "<") {
            ast_node next_node = parse_node(lexer);

            if(next_node.type() != ast_node_type::ast_node_valuedecl) {
                token.throw_error_at_current_position("Expected base class name");
            }

            next_node.set_type(ast_node_type::ast_node_declname);

            ast_node base_class_node = ast_node(ast_node_type::ast_node_classdecl_base);
            base_class_node.add_child(next_node);

            class_node.add_child(base_class_node);

            token = lexer.next_token();

            if(token.content() != "{") {
                token.throw_error_at_current_position("Expected '{' after base class name");
            }
        } else {
            if(token.content() != "{") {
                token.throw_error_at_current_position("Expected '{' after class name");
            }
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

        ast_node value_node = extract_value(lexer);

        var_node.add_child(value_node);

        token = lexer.next_token();

        return var_node;
    } else if(token.content() == "function") {
        ast_node method_node(ast_node_type::ast_node_fn_decl);
        method_node.add_child(ast_node(std::move(token), ast_node_type::ast_node_decltype));

        token = lexer.next_token();

        if(token.type() != lexer::token_type::token_identifier) {
            // new is a keyword, but it is not a valid method name (constructor)
            if(token.type() == lexer::token_type::token_keyword && token.content() != "new") {
                token.throw_error_at_current_position("Expected method name after 'function'");
            }
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

        return_node.add_child(extract_value(lexer));

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
    } else if(token.content() == "new") {
        uva::lang::lexer::token class_name_token = extract_identifier(lexer);

        ast_node object_node(ast_node_type::ast_node_fn_object);
        object_node.add_child(std::move(ast_node(std::move(class_name_token), ast_node_type::ast_node_declname)));

        ast_node fn_node(ast_node_type::ast_node_fn_call);
        fn_node.add_child(std::move(ast_node(std::move(token), ast_node_type::ast_node_declname)));

        token = lexer.next_token();

        if(token.content() != "(") {
            token.throw_error_at_current_position("Expected '(' after 'new'");
        }

        ast_node params = extract_fn_call_params(lexer);
        fn_node.add_child(std::move(params));

        fn_node.add_child(std::move(object_node));

        return fn_node;
    } else if(token.content() == "require") {
        lexer.rollback_token();

        ast_node require_node = extract_fn_call(lexer);

        ast_node* params = require_node.child_from_type(ast_node_type::ast_node_fn_params);

        if(params == nullptr || params->childrens().empty()) {
            token.throw_error_at_current_position("Expected file name after 'require'");
        }

        const uva::lang::lexer::token& file_name_token = params->childrens().front().token();

        if(file_name_token.type() != lexer::token_type::token_literal || file_name_token.kind() != lexer::token_kind::token_string) {
            file_name_token.throw_error_at_current_position("Expected string literal");
        }

        const std::string& file_path_string = file_name_token.content();

        auto files = list_files_with_wildcard(std::filesystem::current_path(), file_path_string);
        ast_node expansion_node = ast_node(ast_node_type::ast_node_expansion);

        for(const std::string& file : files) {
            std::string file_content = uva::file::read_all_text<char>(file);
            uva::lang::lexer l(file, file_content);

            expansion_node.add_child(std::move(parse_all(l)));
        }

        return expansion_node;
    }
    
    token.throw_error_at_current_position("Unexpected keyword");

    return ast_node();
}

uva::lang::parser::ast_node uva::lang::parser::parse_identifier(uva::lang::lexer &lexer)
{
    // First check if it has '(' after the identifier

    const uva::lang::lexer::token& identifier = lexer.next_token();
    const uva::lang::lexer::token& next_token = lexer.next_token();

    if(next_token.content() == "(") {
        lexer.rollback_token(); // Rollback the '(' token
        lexer.rollback_token(); // Rollback the identifier token
        
        ast_node method_node = extract_fn_call(lexer);

        return method_node;
    }
    else if(next_token.content() == ".") {
        ast_node next_node = parse_node(lexer);

        if(next_node.type() == ast_node_type::ast_node_valuedecl) {
            return ast_node(uva::lang::lexer::token(identifier.start, next_node.token().end, identifier.content() + next_token.content() + next_node.token().content(), uva::lang::lexer::token_type::token_identifier), ast_node_type::ast_node_valuedecl);
        }

        if(next_node.type() != ast_node_type::ast_node_fn_call) {
            next_token.throw_error_at_current_position("Expected function call after '.'");
        }
        
        ast_node object_node(ast_node_type::ast_node_fn_object);
        object_node.add_child(std::move(ast_node(std::move(identifier), ast_node_type::ast_node_declname)));

        next_node.add_child(std::move(object_node));

        return next_node;
    }
    else if(next_token.content() == "!" || next_token.content() == "?") {
        // ! or ? after the identifier is a method call

        ast_node method_node(ast_node_type::ast_node_fn_call);

        uva::lang::lexer::token name_token(identifier.start, next_token.end, identifier.content() + next_token.content(), lexer::token_type::token_identifier);

        method_node.add_child(std::move(ast_node(std::move(name_token), ast_node_type::ast_node_declname)));

        // The parser separates the identifier from the '!' token.
        // In this case, the () is not required

        uva::lang::lexer::token token = lexer.next_token();

        if(token.content() == "(") {
            ast_node params_node = extract_fn_call_params(lexer);
            method_node.add_child(std::move(params_node));
        } else {
            lexer.rollback_token();
        }

        return method_node;
    } else if(next_token.type() == uva::lang::lexer::token_operator) {
        // An identifier + operator

        ast_node node(ast_node_type::ast_node_fn_call);
        node.add_child(std::move(ast_node(std::move(next_token), ast_node_type::ast_node_declname)));

        ast_node object_node(ast_node_type::ast_node_fn_object);
        object_node.add_child(std::move(ast_node(std::move(identifier), ast_node_type::ast_node_declname)));

        node.add_child(std::move(object_node));

        ast_node right_node = parse_node(lexer);

        ast_node params_node = ast_node(ast_node_type::ast_node_fn_params);
        params_node.add_child(std::move(right_node));

        node.add_child(std::move(params_node));

        // Check if the operator is '[', in this case, we need to close it with ']'

        ast_node* declname = node.child_from_type(ast_node_type::ast_node_declname);

        if(declname->token().content() == "[") {
            uva::lang::lexer::token token = lexer.next_token();

            if(token.content() != "]") {
                token.throw_error_at_current_position("Expected ']' after '[]'");
            }

            declname->set_token(uva::lang::lexer::token(declname->token().start, declname->token().end, "[]", uva::lang::lexer::token_type::token_operator));
        }

        // Check if we have a chain of operators

        uva::lang::lexer::token token = lexer.next_token();

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
        lexer.rollback_token();

        ast_node node(std::move(identifier), ast_node_type::ast_node_valuedecl);
        return node;
    }

    identifier.throw_error_at_current_position("Unexpected identifier");

    return ast_node();
}

uva::lang::parser::ast_node uva::lang::parser::parse_literal(uva::lang::lexer &lexer)
{
    const uva::lang::lexer::token& token = lexer.next_token();
    return ast_node(std::move(token), ast_node_type::ast_node_valuedecl);
}

uva::lang::parser::ast_node uva::lang::parser::parse_delimiter(uva::lang::lexer &lexer)
{
    const uva::lang::lexer::token& token = lexer.next_token();

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
}

uva::lang::parser::ast_node uva::lang::parser::parse_operator(uva::lang::lexer &lexer)
{
    uva::lang::lexer::token token = lexer.next_token();

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

uva::lang::parser::ast_node uva::lang::parser::parse_eof(uva::lang::lexer &lexer)
{
    uva::lang::lexer::token token = lexer.next_token();
    return ast_node(std::move(token), ast_node_type::ast_node_undefined);
}
