#include <lexer.hpp>

// std::vector<std::pair<std::string_view, uva::lang::lexer::cursor_type>> uva::lang::lexer::cursor_type_from_string_map = {
//     /* Keep in most used order */

//     { "//",       cursor_type::cursor_comment          },
//     { "var",      cursor_type::cursor_var              },
//     { "{",        cursor_type::cursor_block            },
//     { "function", cursor_type::cursor_function         },
//     { "return",   cursor_type::cursor_return           },
//     { "class",    cursor_type::cursor_class            },
//     { "extends",  cursor_type::cursor_baseclass        },
//     { "require_directory", cursor_type::cursor_fncall  },
// };

bool is_delimiter(const char& c) {
    static std::vector<bool> delimiters;
    if(delimiters.empty()) {
        delimiters.resize(256, false);

        delimiters['{'] = true;
        delimiters['}'] = true;
        delimiters['('] = true;
        delimiters[')'] = true;
        delimiters['['] = true;
        delimiters[']'] = true;
        delimiters[';'] = true;
        delimiters[','] = true;
        delimiters['.'] = true;
        delimiters[':'] = true;
    }

    return delimiters[c];
}

bool is_operator(const char& c) {
    static std::vector<bool> operators;
    if(operators.empty()) {
        operators.resize(256, false);

        operators['+'] = true;
        operators['-'] = true;
        operators['*'] = true;
        operators['/'] = true;
        operators['%'] = true;
        operators['='] = true;
        operators['!'] = true;
        operators['<'] = true;
        operators['>'] = true;
    }

    return operators[c];
}

bool is_keyword(const std::string& str) {
    static std::map<std::string, bool> keywords;

    if(keywords.empty()) {
        keywords["var"]      = true;
        keywords["function"] = true;
        keywords["return"]   = true;
        keywords["class"]    = true;
    }

    return keywords.find(str) != keywords.end();
}

uva::lang::lexer::lexer(const std::string &__file_name, const std::string_view &__source)
  : m_file_name(__file_name), m_source(__source)
{
    tokenize();
}

void uva::lang::lexer::update_start_position(const char &token)
{
    if(token == '\n') {
        m_start.line++;
        m_start.column = 0;
    } else {
        m_start.column++;
    }

    m_start.offset++;
}

const char &uva::lang::lexer::discard()
{
    const char& c = m_source.front();

    update_start_position(c);

    m_source.remove_prefix(1);

    return c;
}


void uva::lang::lexer::discard_whitespaces()
{
    discard_while([](const char& c) {
        return isspace(c);
    });
}

const char &uva::lang::lexer::read()
{
    // If it needs to read a character, and the buffer is empty, it is an error.
    if(m_source.empty()) {
        throw std::runtime_error("lexer: unexpected end of file");
    }
    
    const char& c = m_source.front();

    update_start_position(c);

    m_source.remove_prefix(1);

    m_buffer.push_back(c);

    return c;
}

uva::lang::lexer::token uva::lang::lexer::read_next_token()
{
    m_buffer.clear();

    // First, make sure we are at the beginning of the source code, not line breaks or spaces.
    discard_whitespaces();

    token_position start = m_start;

    if(m_source.empty()) {
        return token(m_start, m_start, "", token_type::token_eof);
    }

    const char& c = m_source.front();

    // Ordered by less expensive checks first

    if(is_delimiter(c)) {
        read();
        return uva::lang::lexer::token(start, m_start, m_buffer, token_type::token_delimiter);
    }

    // The comment starts with /, which is the division operator. So we need to check if it is a comment first.
    if(c == '/' && m_source.size() > 2 && m_source[1] == '/') {
        discard();
        discard();

        read_while([](const char& c) {
            return c != '\n';
        });

        return uva::lang::lexer::token(start, m_start, m_buffer, token_type::token_comment);
    }

    if(is_operator(c)) {
        read();
        return uva::lang::lexer::token(start, m_start, m_buffer, token_type::token_operator);
    }

    if(c == '\"') {
        discard();

        read_while([](const char& c) {
            return c != '\"';
        });

        discard();

        return uva::lang::lexer::token(start, m_start, m_buffer, token_type::token_literal, token_kind::token_string);
    }

    if(isdigit(c)) {
        // if a token starts with a digit, it is a number
        read_while([](const char& c) {
            return isdigit(c);
        });

        return uva::lang::lexer::token(start, m_start, m_buffer, token_type::token_literal, token_kind::token_integer);
    }

    // It must be a identifier or a keyword
    read_while([](const char& c) {
        return isalnum(c) || c == '_';
    });

    // Todo: map
    if(m_buffer == "null") {
        return uva::lang::lexer::token(start, m_start, m_buffer, token_type::token_literal, token_kind::token_null);
    } else if(m_buffer == "false") {
        return uva::lang::lexer::token(start, m_start, m_buffer, token_type::token_literal, token_kind::token_boolean);
    } else if(m_buffer == "true") {
        return uva::lang::lexer::token(start, m_start, m_buffer, token_type::token_literal, token_kind::token_boolean);
    }

    if(is_keyword(m_buffer)) {
        return uva::lang::lexer::token(start, m_start, m_buffer, token_type::token_keyword);
    } else {
        return uva::lang::lexer::token(start, m_start, m_buffer, token_type::token_identifier);
    }

    throw std::runtime_error("lexer: unknown token");
}

void uva::lang::lexer::tokenize()
{
    uva::lang::lexer::token token = read_next_token();

    do {
        m_tokens.push_back(token);
        token = read_next_token();
    } while(!token.is_eof());

    m_tokens.push_back(token);
}

const uva::lang::lexer::token& uva::lang::lexer::next_token()
{
    if(iterator >= m_tokens.size()) {
        // If the parser is trying to get a token that does not exist, it is an error.
        throw std::runtime_error("unexpected end of file");
    }

    return m_tokens[iterator++];
}

const uva::lang::lexer::token& uva::lang::lexer::previous_token()
{
    if(iterator == 0) {
        throw std::runtime_error("unexpected begin of file");
    }

    --iterator;

    return m_tokens[iterator - 1];
}

uva::lang::lexer::token::token(token_position start, token_position end, std::string content, token_type type)
    : start(start), end(end), m_content(std::move(content)), m_type(type)
{
}

uva::lang::lexer::token::token(token_position start, token_position end, std::string content, token_type type, token_kind kind)
    : start(start), end(end), m_content(std::move(content)), m_type(type), m_kind(kind)
{

}

void uva::lang::lexer::token::throw_error_at_current_position(std::string what) const
{
    what += " at ";
    what += human_start_position();
    throw std::runtime_error(what);
}

void uva::lang::lexer::token::throw_unexpected_eof_if_is_eol() const
{
    if(is_eof()) {
        std::string message = "Unexpected end of file";
        throw_error_at_current_position(std::move(message));
    }
}

std::string_view uva::lang::lexer::token::human_start_position() const
{
    static std::string result;
    result.clear();

    result += std::to_string(start.line+1);
    result.push_back(':');
    result += std::to_string(start.column+1);

    return result;
}
