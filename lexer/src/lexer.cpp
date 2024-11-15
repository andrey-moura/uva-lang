#include <lexer.hpp>

#include <algorithm>

bool is_delimiter(const char& c) {
    static std::vector<bool> delimiters;
    if(delimiters.empty()) {
        delimiters.resize(256, false);

        delimiters['{'] = true;
        delimiters['}'] = true;
        delimiters['('] = true;
        delimiters[')'] = true;
        delimiters[';'] = true;
        delimiters[','] = true;
        delimiters[':'] = true;
    }

    return delimiters[(uint8_t)c];
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
        operators['?'] = true;
        operators['<'] = true;
        operators['>'] = true;
        operators['['] = true;
        operators[']'] = true;
        operators['|'] = true;
        operators['&'] = true;
        operators['.'] = true;
    }

    return operators[(uint8_t)c];
}

bool is_keyword(const std::string& str) {
    static std::vector<std::string> keywords;

    if(keywords.empty()) {
        keywords = {
            "var",
            "function",
            "return",
            "class",
            "if",
            "foreach",
            "new",
            "else"
        };

        std::sort(keywords.begin(), keywords.end());
    }

    return std::binary_search(keywords.begin(), keywords.end(), str);
}

bool is_preprocessor(std::string_view str) {
    if(str.starts_with('#')) {
        return true;
    }
    static std::vector<std::string> preprocessor;

    if(preprocessor.empty()) {
        preprocessor = {
            "#include",
            "#define",
            "#ifdef",
            "#endif",
        };

        std::sort(preprocessor.begin(), preprocessor.end());
    }

    return std::binary_search(preprocessor.begin(), preprocessor.end(), str);
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

void uva::lang::lexer::push_token(token_position start, token_type type, std::string content, token_kind kind)
{
    m_tokens.emplace_back(start, m_start, std::move(content), type, kind, m_file_name);
}

char unescape(const char& c)
{
    switch(c) {
        case '\\':
        case '"':
        case '\'':
            return c;
        case 'n':
            return '\n';
        case 't':
            return '\t';
        case 'r':
            return '\r';
        case 'b':
            return '\b';
        case 'a':
            return '\b';
        case 'f':
            return '\f';
        case 'v':
            return '\v';
        default:
            break;
    }

    throw std::runtime_error("lexer: cannot unescape '" + std::string(1, c) + "'");
}

void uva::lang::lexer::read_next_token()
{
    m_buffer.clear();

    // First, make sure we are at the beginning of the source code, not line breaks or spaces.
    discard_whitespaces();

    token_position start = m_start;

    if(m_source.empty()) {
        push_token(start, token_type::token_eof);
        return;
    }

    const char& c = m_source.front();

    // Ordered by less expensive checks first

    if(is_delimiter(c)) {
        read();
        push_token(start, token_type::token_delimiter, std::move(m_buffer));
        return;
    }

    // The comment starts with /, which is the division operator. So we need to check if it is a comment first.
    if(c == '/' && m_source.size() > 2 && m_source[1] == '/') {
        discard();
        discard();

        read_while([](const char& c) {
            return c != '\n';
        });

        push_token(start, token_type::token_comment, std::move(m_buffer));
        return;
    }

    if(isdigit(c) || (c == '-' && isdigit(m_source[1]))) {
        // if a token starts with a digit or a minus sign followed by a digit, it is a number
        read_while([this](const char& c) {
            return isdigit(c) || (m_buffer.empty() && c == '-');
        });

        push_token(start, token_type::token_literal, std::move(m_buffer), token_kind::token_integer);
        return;
    }

    if(is_operator(c)) {
        read();

        // If the next character is also an operator, it is a double operator.
        if(is_operator(m_source.front())) {
            read();
        }

        push_token(start, token_type::token_operator, std::move(m_buffer));
        return;
    }

    if(c == '\"') {
        discard();

        extract_and_push_string(start);
        return;
    }

    if(is_preprocessor(m_source)) {
        read_while([](const char& c) {
            return !isspace(c);
        });

        push_token(start, token_type::token_preprocessor, std::move(m_buffer));
        return;
    }

    // It must be a identifier or a keyword
    read_while([](const char& c) {
        return isalnum(c) || c == '_';
    });

    if(m_buffer.empty()) {
        push_token(start, token_type::token_undefined);
        return;
    }

    // Todo: map
    if(m_buffer == "null") {
        push_token(start, token_type::token_literal, std::move(m_buffer), token_kind::token_null);
        return;
    } else if(m_buffer == "false") {
        push_token(start, token_type::token_literal, std::move(m_buffer), token_kind::token_boolean);
        return;
    } else if(m_buffer == "true") {
        push_token(start, token_type::token_literal, std::move(m_buffer), token_kind::token_boolean);
        return;
    }

    if(is_keyword(m_buffer)) {
        push_token(start, token_type::token_keyword, std::move(m_buffer));
        return;
    } else {
        push_token(start, token_type::token_identifier, std::move(m_buffer));
        return;
    }

    throw std::runtime_error("lexer: unknown token");
}

void uva::lang::lexer::tokenize()
{
    do {
        read_next_token();
    } while(!m_tokens.back().is_eof());
}

void uva::lang::lexer::consume_token()
{
    if(!has_next_token()) {
        // If the parser is trying to get a token that does not exist, it is an error.
        throw std::runtime_error("unexpected end of file");
    }

    iterator++;
}

uva::lang::lexer::token &uva::lang::lexer::next_token()
{
    uva::lang::lexer::token& token = m_tokens[iterator];
    consume_token();
    return token;
}

const uva::lang::lexer::token &uva::lang::lexer::see_next()
{
    if(!has_next_token()) {
        throw std::runtime_error("unexpected end of file");
    }

    return m_tokens[iterator];
}

const uva::lang::lexer::token& uva::lang::lexer::previous_token()
{
    if(!has_previous_token()) {
        throw std::runtime_error("unexpected begin of file");
    }

    --iterator;

    return m_tokens[iterator - 1];
}

void uva::lang::lexer::rollback_token()
{
    if(iterator < 1) {
        throw std::runtime_error("unexpected begin of file");
    }

    iterator--;
}

void uva::lang::lexer::erase_tokens(size_t count)
{
    if(iterator + count > m_tokens.size()) {
        throw std::runtime_error("unexpected end of file");
    }

    m_tokens.erase(m_tokens.begin() + iterator - 1, m_tokens.begin() + iterator + count - 1);

    iterator--;
}

void uva::lang::lexer::erase_eof()
{
    if(m_tokens.back().is_eof()) {
        m_tokens.pop_back();
    }
}

void uva::lang::lexer::insert(const std::vector<uva::lang::lexer::token> &tokens)
{
    m_tokens.insert(m_tokens.begin() + iterator, tokens.begin(), tokens.end());
    iterator += tokens.size();
}

uva::lang::lexer::token::token(token_position start, token_position end, std::string content, token_type type, token_kind kind, std::string file_name)
    : start(start), end(end), m_content(std::move(content)), m_type(type), m_kind(kind), m_file_name(std::move(file_name))
{
}

uva::lang::lexer::token::token(token_position start, token_position end, std::string content, token_type type, token_kind kind)
    : start(start), end(end), m_content(std::move(content)), m_type(type), m_kind(kind)
{

}

uva::lang::lexer::token::token(token &&other)
    : start(other.start), end(other.end), m_content(std::move(other.m_content)), m_type(other.m_type), m_kind(other.m_kind), m_file_name(std::move(other.m_file_name))
{
}

std::string uva::lang::lexer::token::error_message_at_current_position(std::string_view what) const
{
    std::string output(what);
    output += " at ";
    output += human_start_position();
    
    return output;
}

std::string uva::lang::lexer::token::unexpected_eof_message() const
{
    return error_message_at_current_position("unexpected end of file");
}

std::string_view uva::lang::lexer::token::human_start_position() const
{
    static std::string result;
    result.clear();

    result += m_file_name;
    result.push_back(':');
    result += std::to_string(start.line+1);
    result.push_back(':');
    result += std::to_string(start.column+1);

    return result;
}

void uva::lang::lexer::token::merge(const token &other)
{
    m_content += other.m_content;
    end = other.end;
}

void uva::lang::lexer::extract_and_push_string(token_position start)
{
    while(m_source.size()) {
        char ch = m_source.front();

        switch(ch)
        {
            case '\\':
                m_source.remove_prefix(1); // Remove the backslash
                ch = m_source.front();     // Save the escaped character
                m_source.remove_prefix(1); // Remove the escaped character

                m_start.offset += 2;        // The backslash and the escaped character
                m_start.column += 2;        // The backslash and the escaped character

                m_buffer.push_back(unescape(ch));
            break;
            case '\"':
                discard();
                push_token(start, token_type::token_literal, std::move(m_buffer), token_kind::token_string);
                return;
            break;
            case '$':
                if(m_source.size() > 1 && m_source[1] == '{') {
                    discard(); // Remove the dollar sign
                    discard(); // Remove the opening curly brace open

                    // Push the string before the variable or expression
                    push_token(start, token_type::token_literal, std::move(m_buffer), token_kind::token_string);

                    // We call the operator + to concatenate the string with the variable or expression 
                    push_token(start, token_type::token_operator, "+", token_kind::token_string);

                    // Read the variable or expression
                    while(m_source.size() && m_source.front() != '}') {
                        read_next_token();
                    }

                    if(m_source.size()) {
                        discard(); // Remove the closing curly brace
                    }
                    
                    // Check if the string is finished
                    if(m_source.size() && m_source.front() == '\"') {
                        discard(); // Remove the closing quote
                        return;
                    }

                    // We call the operator + to concatenate the string with the variable or expression 
                    push_token(m_start, token_type::token_operator, "+", token_kind::token_string);

                    // Read the continuation of the string after the variable or expression
                    extract_and_push_string(m_start);
                    return;
                }

                read();
            break;
            default:
                read();
                break;
        }
    }

    throw std::runtime_error("lexer: unexpected end of file");
}