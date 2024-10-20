#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <map>

namespace uva
{
    namespace lang
    {
        namespace lexer {
            enum cursor_type {
                cursor_undefined,
                cursor_comment,
                cursor_block,
                cursor_function,
                cursor_return,
                cursor_decname,
                cursor_decfnparams,
                cursor_dectype,
                cursor_class,
                cursor_baseclass,
                cursor_value,
                cursor_fncall,
                cursor_fncallparams,
                cursor_eof
            };
            struct cursor_position {
                size_t line = 0;
                size_t column = 0;
                size_t offset = 0;
            };
            class cursor
            {
            public:
                cursor(std::string_view source);
                cursor() = default;
            public:
                const cursor_type& type() const {
                    return m_type;
                }
                bool eof() const {
                    return m_type == cursor_type::cursor_eof;
                }
                const std::string_view& content() const {
                    return m_content;
                }
                const uva::lang::lexer::cursor_position& start() const {
                    return m_start;
                }
                const uva::lang::lexer::cursor_position& end() const {
                    return m_end;
                }
                std::string human_start_position() const {
                    std::string result;
                    result += std::to_string(m_start.line+1);
                    result.push_back(':');
                    result += std::to_string(m_start.column+1);
                    return result;
                }
                std::string human_end_position() const {
                    std::string result;
                    result += std::to_string(m_end.line+1);
                    result.push_back(':');
                    result += std::to_string(m_end.column+1);
                    return result;
                }
            private:
                std::string_view m_source;
                std::string_view m_buffer;
                std::string_view m_content;

                uva::lang::lexer::cursor_position m_start;
                uva::lang::lexer::cursor_position m_end;

                std::vector<uva::lang::lexer::cursor> m_children;

                cursor_type m_type = cursor_type::cursor_undefined;

                static std::map<uva::lang::lexer::cursor_type, void(uva::lang::lexer::cursor::*)()> s_lexers;
            //parsing
            protected:
                void parse();
                uva::lang::lexer::cursor init_next();
                const char& discard();
                const char& extend();
                /// @brief Push a character from buffer to content untill the buffer starts with the token or eof. The token itself is not pushed to the content.
                /// @param token The token which should stop the extending
                /// @return The last character pushed to the content.
                const char& extend_untill_token_or_eof(const char& token);
                /// @brief Push a character from buffer to content untill the buffer starts with the token. If EOF is found, a exception is thrown. The token itself is not pushed to the content.
                /// @param token The token which should stop the extending
                /// @return The last character pushed to the content.
                const char& extend_untill_token(const char& token);
                void discard_whitespaces();
                void extend_whitespaces();
                void update_position(uva::lang::lexer::cursor_position& position, const char& token);
                void update_start_position(const char& token);
                void update_end_position(const char& token);

                void throw_error_at_current_position(std::string what);
                void throw_unexpected_token_at_current_position(const char& token);
                void throw_unexpected_eof();
                void throw_unexpected_eof_if_buffer_is_empty();

                void extend_by(const uva::lang::lexer::cursor& cursor);
                void extend_by_last_child_if_exists();

                void lexer_comment();
                void lexer_block();
                void lexer_function();
                void lexer_return();
                void lexer_class();
                void lexer_dectype();
                void lexer_decname();
                void lexer_decfnparams();
                void lexer_value();
                void lexer_fncall();
                void lexer_fncallparams();
                void lexer_undefined();
            //parsing public interface
            public:
                uva::lang::lexer::cursor parse_next();

                const uva::lang::lexer::cursor& child_from_type(const cursor_type& __type) const {
                    for(auto& child : m_children) {
                        if(child.type() == __type) {
                            return child;
                        }
                    }

                    throw std::runtime_error("child not found");
                }

                const std::string_view& child_content_from_type(const cursor_type& __type) const {
                    return child_from_type(__type).content();
                }

                const std::string_view& decname() const {
                    return child_content_from_type(cursor_type::cursor_decname);
                }

                /// @brief The value of the cursor. Available only for cursor_type::cursor_return
                /// @return a std::string_view containing the raw value of the cursor read from the source
                const std::string_view& value() const {
                    return child_content_from_type(cursor_type::cursor_value);
                }

                const uva::lang::lexer::cursor& block() const {
                    return child_from_type(cursor_type::cursor_block);
                }

                const uva::lang::lexer::cursor& fncallparams() const {
                    return child_from_type(cursor_type::cursor_fncallparams);
                }

                const std::vector<uva::lang::lexer::cursor>& children() const {
                    return m_children;
                }

                void throw_error_at_current_position(std::string what) const;
                void throw_unexpected_token_at_current_position(const char& token) const;
                void throw_unexpected_eof() const;
                void throw_unexpected_eof_if_buffer_is_empty() const;
            };
        };
    };
}; // namespace uva