#pragma once

#include <vector>
#include <memory>
#include <map>
#include <utility>
#include <string>
#include <stdexcept>

namespace uva
{
    namespace lang
    {
        class lexer
        {
        public:
            lexer(const std::string& __file_name, const std::string_view& __source);
            ~lexer() = default;
        public:
            enum token_type {
                token_undefined,
                token_comment,
                token_keyword,
                token_identifier,
                token_literal,
                token_delimiter,
                token_operator,
                token_eof,
                token_type_max
            };
            enum token_kind {
                token_null,
                token_boolean,
                token_integer,
                token_float,
                token_string,
            };
            struct token_position {
                size_t line = 0;
                size_t column = 0;
                size_t offset = 0;
            };
            class token {
            protected:
                std::string m_content;
                token_type m_type;
                token_kind m_kind;
            public:
                std::string_view m_file_name;
            public:
                token(token_position start, token_position end, std::string content, token_type type);
                token(token_position start, token_position end, std::string content, token_type type, token_kind kind);
                token() = default;
                ~token() = default;
            public:
            public:
                bool is_eof() const { return m_type == token_type::token_eof; }
            public:
                void throw_error_at_current_position(std::string what) const;
                void throw_unexpected_eof_if_is_eol() const;
                std::string_view human_start_position() const;

                void merge(const token& other);
            public:
                /// @brief Return the content of the token.
                const std::string & content() const { return m_content; }
                /// @brief Return the type of the token.
                token_type type() const { return m_type; }
                /// @brief Return the kind of the token.
                token_kind kind() const { return m_kind; }
            public:
                token_position start;
                token_position end;
            };
        protected:
            std::string m_file_name;
            std::string_view m_source;
            std::string m_buffer;
            std::vector<uva::lang::lexer::token> m_tokens;

            token_position m_start;

            // iterating
            size_t iterator = 0;
        protected:
            /// @brief Update a position (line, column, offset).
            /// @param position The position to update.
            void update_position(token_position& position, const char& token);
            /// @brief Update the start position (line, column, offset).
            /// @param token The token which should update the position.
            void update_start_position(const char& token);

            /// @brief Discard the first character from the m_source and update the start position.
            const char& discard();
            /// @brief Discard all whitespaces from the m_source.
            void discard_whitespaces();

            /// @brief Read the first character from the m_source, stores it in m_buffer and update the start position.
            const char& read();

            template<typename T>
            void discard_while(T&& condition) {
                while(m_source.size() && condition(m_source.front())) {
                    discard();
                }
            }

            template<typename T>
            void read_while(T&& condition) {
                while(m_source.size() && condition(m_source.front())) {
                    read();
                }
            }

            uva::lang::lexer::token read_next_token();
            void tokenize();
        // iterating
        public:
            /// @brief Increment the iterator
            void consume_token();
            /// @brief Return the next token and increment the iterator.
            /// @return The next token.
            const uva::lang::lexer::token& next_token();
            /// @brief Return the next token without incrementing the iterator.
            const uva::lang::lexer::token& see_next();
            /// @brief Decrement the iterator and return the next token.
            /// @return The previous token.
            const uva::lang::lexer::token& previous_token();
            bool has_previous_token() const { return iterator > 0; }
            /// @brief Rollback the token iterator. The next call to next_token will return the same token.
            void rollback_token();
            /// @brief Check if there is a next token.
            bool has_next_token() const { return iterator < m_tokens.size(); }
            /// @brief Reset the iterator to 0.
            void reset() { iterator = 0; }
        protected:
        public:
            //extern std::vector<std::pair<std::string_view, uva::lang::lexer::cursor_type>> cursor_type_from_string_map;
        };
    };
}; // namespace uva