#include <uva.hpp>

#include <preprocessor/preprocessor.hpp>

#include <uva/file.hpp>

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
#ifdef __UVA_WIN__
            std::replace(filename.begin(), filename.end(), '\\', '/');
#endif
            // Verifica se o arquivo corresponde ao padrão
            if (std::regex_match(filename, regex_pattern)) {
                files.push_back(filename);
            }
        }
    }

    return files;
}

std::map<std::string, void(uva::lang::preprocessor::*)(const std::filesystem::path&, uva::lang::lexer&), std::less<>> preprocessor_directives = {
    { "#include", &uva::lang::preprocessor::process_include },
    { "#vm",      &uva::lang::preprocessor::process_vm      },
};

uva::lang::preprocessor::preprocessor(std::filesystem::path __executable_path)
    : m_executable_path(__executable_path)
{
}

uva::lang::preprocessor::~preprocessor()
{
}

void uva::lang::preprocessor::process(const std::filesystem::path &__file_name, uva::lang::lexer &__lexer)
{
    // Now we have a rule defined: The preprocessors must be at the beginning of the file.
    // The preprocessor will stop executing when it finds a token that is not a preprocessor (and is not a comment).

    uva::lang::lexer::token token = __lexer.next_token();

    while(!token.is_eof()) {
        switch(token.type()) {
            case uva::lang::lexer::token_type::token_comment:
                // Do nothing
            break;
            case uva::lang::lexer::token_type::token_preprocessor: {
                if(auto it = preprocessor_directives.find(token.content()); it != preprocessor_directives.end()) {
                    (this->*it->second)(__file_name, __lexer);
                } else {
                    throw std::runtime_error(token.error_message_at_current_position("unknown preprocessor directive"));
                }
            }
                break;
            default:
                __lexer.reset();
                return;
        }

        token = __lexer.next_token();
    }
}

void uva::lang::preprocessor::process_include(const std::filesystem::path &__file_name, uva::lang::lexer &__lexer)
{
    // Moves becase it will be removed
    uva::lang::lexer::token directive       = std::move(__lexer.current_token());
    uva::lang::lexer::token file_name_token = std::move(__lexer.see_next());

    if(file_name_token.type() != lexer::token_type::token_literal || file_name_token.kind() != lexer::token_kind::token_string) {
        throw std::runtime_error(file_name_token.error_message_at_current_position("Expected string literal after include directive"));
    }

    const std::string& file_path_string = file_name_token.content();

    auto files = list_files_with_wildcard(std::filesystem::current_path(), file_path_string);

    __lexer.erase_tokens(2); // Remove the directive and the file name token

    // After this the iterator is at the position of the next token
    
    for(const std::string& file : files) {
        std::string file_content = uva::file::read_all_text<char>(file);
        uva::lang::lexer l(file, file_content);

        process(file, l);

        l.erase_eof();

        __lexer.insert(l.tokens());
    }
}

void uva::lang::preprocessor::process_vm(const std::filesystem::path &__file_name, uva::lang::lexer &__lexer)
{
    // Moves becase it will be removed
    uva::lang::lexer::token directive       = std::move(__lexer.current_token());
    uva::lang::lexer::token file_name_token = std::move(__lexer.see_next());

    if(file_name_token.kind() != uva::lang::lexer::token_kind::token_string) {
        throw std::runtime_error(file_name_token.error_message_at_current_position("Expected string after boot"));
    }

    __lexer.erase_tokens(2); // Remove the directive and the file name token

    m_vm = file_name_token.content();
}

int uva::lang::preprocessor::launch_vm(int argc, char **argv)
{
    std::filesystem::path boot_path = m_executable_path;
    boot_path.replace_filename(m_vm);

    if(!std::filesystem::exists(boot_path)) {
        throw std::runtime_error("Specified VM does not exist. Expected to find it at" + boot_path.string());
    }

    std::string boot_command = boot_path.string();

    for(int i = 1; i < argc; i++) {
        boot_command += " ";
        boot_command += argv[i];
    }

    return system(boot_command.c_str());
}
