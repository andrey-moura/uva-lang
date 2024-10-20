#include <lexer/cursor.hpp>
#include <lexer/lexer.hpp>

#include <ctype.h>
#include <string.h>

std::map<uva::lang::lexer::cursor_type, void(uva::lang::lexer::cursor::*)()> uva::lang::lexer::cursor::s_lexers = {
    { uva::lang::lexer::cursor_type::cursor_comment,      &uva::lang::lexer::cursor::lexer_comment },
    { uva::lang::lexer::cursor_type::cursor_block,        &uva::lang::lexer::cursor::lexer_block },
    { uva::lang::lexer::cursor_type::cursor_function,     &uva::lang::lexer::cursor::lexer_function },
    { uva::lang::lexer::cursor_type::cursor_return,       &uva::lang::lexer::cursor::lexer_return },
    { uva::lang::lexer::cursor_type::cursor_class,        &uva::lang::lexer::cursor::lexer_class },
    { uva::lang::lexer::cursor_type::cursor_dectype,      &uva::lang::lexer::cursor::lexer_dectype },
    { uva::lang::lexer::cursor_type::cursor_decname,      &uva::lang::lexer::cursor::lexer_decname },
    { uva::lang::lexer::cursor_type::cursor_decfnparams,  &uva::lang::lexer::cursor::lexer_decfnparams },
    { uva::lang::lexer::cursor_type::cursor_value,        &uva::lang::lexer::cursor::lexer_value },
    { uva::lang::lexer::cursor_type::cursor_fncallparams, &uva::lang::lexer::cursor::lexer_fncallparams },
    { uva::lang::lexer::cursor_type::cursor_undefined,    &uva::lang::lexer::cursor::lexer_undefined },
};

uva::lang::lexer::cursor::cursor(std::string_view source)
    : m_source(source), m_buffer(source), m_start(), m_end(), m_type(cursor_type::cursor_undefined)
{
    parse();
}

uva::lang::lexer::cursor uva::lang::lexer::cursor::init_next()
{
    uva::lang::lexer::cursor sc;
    sc.m_source = m_source;
    sc.m_buffer = m_source.substr(m_end.offset);
    sc.m_start  = m_end;
    sc.m_end    = m_end;

    return sc;
}

uva::lang::lexer::cursor uva::lang::lexer::cursor::parse_next()
{
    uva::lang::lexer::cursor sc = init_next();
    sc.parse();

    return sc;
}

const char &uva::lang::lexer::cursor::discard()
{
    const char& c = m_buffer.front();

    update_start_position(c);

    m_buffer.remove_prefix(1);

    const char* start = m_source.begin() + m_start.offset;
    const char* end   = m_source.begin() + m_end.offset;
    
    if(m_end.offset) {
        m_content = std::string_view(start, end - start);
    }

    return c;
}

const char& uva::lang::lexer::cursor::extend()
{
    const char& c = m_buffer.front();

    update_end_position(c);

    m_buffer.remove_prefix(1);
    
    const char* start = m_source.begin() + m_start.offset;
    const char* end   = m_source.begin() + m_end.offset;
    
    m_content = std::string_view(start, end - start);

    return c;
}

const char &uva::lang::lexer::cursor::extend_untill_token_or_eof(const char &token)
{
    while(m_buffer.size() && m_buffer.front() != token) {
        extend();
    }

    return m_buffer.front();
}

const char &uva::lang::lexer::cursor::extend_untill_token(const char &token)
{
    extend_untill_token_or_eof(token);

    // If buffer does not starts with the token, it means we reached the end of the file.
    if(!m_buffer.starts_with(token)) {
        throw_unexpected_eof();
    }

    return m_buffer.front();
}

void uva::lang::lexer::cursor::discard_whitespaces()
{
    while(m_buffer.size() && (isspace(m_buffer.front()) || m_buffer.starts_with(';'))) {
        discard();
    }
}

void uva::lang::lexer::cursor::extend_whitespaces()
{
    while(m_buffer.size() && (isspace(m_buffer.front()) || m_buffer.starts_with(';'))) {
        extend();
    }
}

void uva::lang::lexer::cursor::update_position(uva::lang::lexer::cursor_position &position, const char &token)
{
    if(m_buffer.front() == '\n') {
        position.line++;
        position.column = 0;
    } else {
        position.column++;
    }

    position.offset++;
}

void uva::lang::lexer::cursor::update_start_position(const char &token)
{
    update_position(m_start, token);
}

void uva::lang::lexer::cursor::update_end_position(const char &token)
{
    update_position(m_end, token);
}

void uva::lang::lexer::cursor::extend_by(const uva::lang::lexer::cursor &cursor)
{
    m_end = cursor.end();
    m_content = std::string_view(m_source.begin() + m_start.offset, m_end.offset - m_start.offset);
    m_buffer  = cursor.m_buffer;
}

void uva::lang::lexer::cursor::extend_by_last_child_if_exists()
{
    if(m_children.size()) {
        extend_by(m_children.back());
    }
}

void uva::lang::lexer::cursor::parse()
{
    //First, make sure we are at the beginning of the source code, not comments or spaces.
    discard_whitespaces();

    m_end = m_start;

    //At this point, we are at the beginning of the source code. The next character may be a comment or a token.

    if(m_buffer.empty()) {
        m_type = cursor_type::cursor_eof;
        return;
    }

    if(m_type == cursor_type::cursor_undefined) {
        // If we don't know what it is, we need to figure out.
        for(auto& [token, type] : uva::lang::lexer::cursor_type_from_string_map) {
            if(m_buffer.starts_with(token)) {
                m_type = type;
                break;
            }
        }
    }

    void(uva::lang::lexer::cursor::*parser)() = s_lexers[m_type];

    ((*this).*parser)();
}

void uva::lang::lexer::cursor::throw_error_at_current_position(std::string what)
{
    what += " at ";
    what += human_start_position();
    throw std::runtime_error(what);
}

void uva::lang::lexer::cursor::throw_unexpected_token_at_current_position(const char &token)
{
    std::string message = "Unexpected token '";
    message.push_back(token);
    message.push_back('\'');
    
    throw_error_at_current_position(std::move(message));
}

void uva::lang::lexer::cursor::throw_unexpected_eof()
{
    std::string message = "Unexpected end of file";
    throw_error_at_current_position(std::move(message));
}

void uva::lang::lexer::cursor::throw_unexpected_eof_if_buffer_is_empty()
{
    if(m_buffer.empty()) {
        throw_unexpected_eof();
    }
}

void uva::lang::lexer::cursor::lexer_comment() {
    // Read the comment
    extend_untill_token_or_eof('\n');
}

void uva::lang::lexer::cursor::lexer_block() {
    extend();
    extend_whitespaces();

    if(m_buffer.starts_with('}')) {
        extend();
    } else {                
        uva::lang::lexer::cursor block_cursor = init_next();
        block_cursor.parse();

        while(!block_cursor.eof()) {
            m_children.push_back(block_cursor);
            block_cursor = block_cursor.parse_next();

            if(block_cursor.type() == cursor_type::cursor_undefined) {
                if(block_cursor.m_buffer.starts_with('}')) {
                    break;
                }
            }
        }

        extend_by(block_cursor);
    }

    if(!m_buffer.ends_with('}')) {
        throw_error_at_current_position("expected '}'");
    }

    extend();
}

void uva::lang::lexer::cursor::lexer_function() {
    uva::lang::lexer::cursor dctype_cursor = init_next();
    dctype_cursor.m_type = cursor_type::cursor_dectype;
    dctype_cursor.parse();
    m_children.push_back(dctype_cursor);

    // Read the function name
    uva::lang::lexer::cursor decname_cursor = dctype_cursor.init_next();
    decname_cursor.m_type = cursor_type::cursor_decname;
    decname_cursor.parse();
    m_children.push_back(decname_cursor);

    if(decname_cursor.content().empty()) {
        throw_error_at_current_position("expected function name");
    }

    // read the params
    uva::lang::lexer::cursor params_cursor = decname_cursor.init_next();
    params_cursor.m_type = cursor_type::cursor_decfnparams;
    params_cursor.parse();
    m_children.push_back(params_cursor);

    // read block
    uva::lang::lexer::cursor block_cursor = params_cursor.init_next();
    block_cursor.parse();
    m_children.push_back(block_cursor);

    if(block_cursor.type() != cursor_type::cursor_block) {
        throw_error_at_current_position("expected '{' after function name");
    }

    m_end = block_cursor.end();
    m_content = std::string_view(m_source.begin() + m_start.offset, m_end.offset - m_start.offset);
}

void uva::lang::lexer::cursor::lexer_return() {
    m_type = cursor_type::cursor_return;

    // Read the return keyword
    while(m_buffer.size() && !isspace(m_buffer.front())) {
        extend();
    }

    // Read the return value
    uva::lang::lexer::cursor return_cursor = init_next();
    return_cursor.m_type = cursor_type::cursor_value;
    return_cursor.parse();
    m_children.push_back(return_cursor);

    extend_by(return_cursor);
}

void uva::lang::lexer::cursor::lexer_class() {
    uva::lang::lexer::cursor dctype_cursor = init_next();
    dctype_cursor.m_type = cursor_type::cursor_dectype;
    dctype_cursor.parse();
    m_children.push_back(dctype_cursor);

    // Read the class name
    uva::lang::lexer::cursor decname_cursor = dctype_cursor.init_next();
    decname_cursor.m_type = cursor_type::cursor_decname;
    decname_cursor.parse();
    m_children.push_back(decname_cursor);

    if(decname_cursor.content().empty()) {
        throw_error_at_current_position("expected class name");
    }

    // read the next token
    uva::lang::lexer::cursor block_cursor = decname_cursor.init_next();
    block_cursor.parse();

    if(block_cursor.content().empty() || block_cursor.type() != cursor_type::cursor_block) {
        throw_error_at_current_position("expected '{' after class name");
    }

    m_children.push_back(block_cursor);

    m_end = block_cursor.end();
    m_content = std::string_view(m_source.begin() + m_start.offset, m_end.offset - m_start.offset);
}

void uva::lang::lexer::cursor::lexer_dectype() {
    // A cursor that represents a declaration type, like class, function, variable, etc.
    while(m_buffer.size() && !isspace(m_buffer.front())) {
        // The type is a single word, so we can just read until we find a space.
        // The symbol need to starts with a letter and can contain letters, numbers,
        // underscores and '::' (which means its from a class or namespace).
        
        const char& c = m_buffer.front();
        
        if(!m_content.size()) {
            // first character must be a letter
            if(!isalpha(c)) {
                throw_unexpected_token_at_current_position(c);
            }
        } else {
            // other characters can be letters, numbers, underscores or '::'

            if(!isalnum(c) && c != '_') {
                if(c == ':' && m_buffer.starts_with("::")) {
                    extend();
                } else {
                    throw_unexpected_token_at_current_position(c);
                }
            }
        }

        extend();
    }
}

void uva::lang::lexer::cursor::lexer_decname() {
    // A cursor that represents a declaration name
    while(m_buffer.size()) {
        // The type is a single word, so we can just read until we find a space.
        // The symbol need to starts with a letter and can contain letters, numbers and underscores.
        
        const char& c = m_buffer.front();

        if(isspace(c)) {
            break;
        }
        
        if(!m_content.size()) {
            // first letter
            if(!isalpha(c)) {
                throw_unexpected_token_at_current_position(c);
            }
        } else if(!isalnum(c) && c != '_') {
            break;
        }

        extend();
    }
}

void uva::lang::lexer::cursor::lexer_decfnparams() {
    // A cursor that represents a declaration function parameters
    if(!m_buffer.starts_with('(')) {
        throw_error_at_current_position("expected '(' after function name");
    }

    while(m_buffer.size()) {
        // Reads () and everything inside it.
        
        const char& c = m_buffer.front();

        extend();

        if(c == ')') {
            break;
        }
    }

    if(!m_content.ends_with(')')) {
        throw_unexpected_eof();
    }
}

void uva::lang::lexer::cursor::lexer_value() {
    // A cursor that represents a value

    throw_unexpected_eof_if_buffer_is_empty();

    if(m_buffer.starts_with('"')) {
        extend();
        extend_untill_token('"');
        extend();
    } else {
        while(m_buffer.size()) {
            // Reads the value until the end of the line or a space.
            
            const char& c = m_buffer.front();

            if(isspace(c) || c == ';') {
                break;
            }

            extend();
        }
    }
}

void uva::lang::lexer::cursor::lexer_fncallparams() {
    // A cursor that represents a function call parameters
    if(!m_buffer.starts_with('(')) {
        throw_error_at_current_position("expected '(' after function name");
    }

    extend();
    extend_whitespaces();

    if(!m_buffer.starts_with(')')) {
        // Currently supports only one parameter
        while(m_buffer.size()) {                  
            uva::lang::lexer::cursor param_cursor = init_next();
            param_cursor.m_type = cursor_type::cursor_value;
            param_cursor.parse();

            if(param_cursor.content().ends_with(':')) {
                // oops, its not a value, its a named parameter
                param_cursor.m_type = cursor_type::cursor_decname;

                uva::lang::lexer::cursor value_cursor = param_cursor.parse_next();
                value_cursor.m_type = cursor_type::cursor_value;
                value_cursor.parse();

                param_cursor.extend_by(value_cursor);

                param_cursor.m_children.push_back(value_cursor);
            }

            m_children.push_back(param_cursor);

            break;
        }

        extend_by_last_child_if_exists();

        if(!m_buffer.starts_with(')')) {
            throw_unexpected_eof();
        }
    }

    extend();
}

void uva::lang::lexer::cursor::lexer_undefined() {
    while(m_buffer.size() && (isalnum(m_buffer.front()) || m_buffer.front() == '_')) {
        extend();
    }

    if(m_buffer.size() && !isspace(m_buffer.front())) {
        // Can be so many things

        switch(m_buffer.front()) {
            case '(': {
                // A function declaration can't be here, so it must be a function call
                m_type = cursor_type::cursor_fncall;

                // The current cursor is the function name.
                uva::lang::lexer::cursor decname_cursor = *this;
                decname_cursor.m_type = cursor_type::cursor_decname;
                m_children.push_back(decname_cursor);

                uva::lang::lexer::cursor params_cursor = decname_cursor.init_next();
                params_cursor.m_type = cursor_type::cursor_fncallparams;
                params_cursor.parse();
                m_children.push_back(params_cursor);

                extend_by(params_cursor);
            }
            break;
        }
    }
}