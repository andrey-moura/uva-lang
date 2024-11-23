#include <filesystem>
#include <chrono>

#include <parser/parser.hpp>
#include <lexer/lexer.hpp>
//#include <vm/vm.hpp>

#include <console.hpp>
#include <uva/file.hpp>
#include <interpreter/interpreter.hpp>
#include <extension/extension.hpp>
#include <preprocessor/preprocessor.hpp>

using namespace uva;

std::shared_ptr<uva::lang::object> application;
std::vector<uva::lang::extension*> extensions;

#ifdef __UVA_DEBUG__
    #define try if(true)
    #define catch(e) if(false)

    std::exception e;
#endif

int main(int argc, char** argv) {
    try {
        auto start = std::chrono::high_resolution_clock::now();

        std::filesystem::path uva_executable_path = argv[0];
        //vm_instance = std::make_shared<uva::lang::vm>();

        std::filesystem::path file_path;
        std::filesystem::path temp_file_path;

        if(argc > 1) {
            file_path = std::filesystem::absolute(argv[1]);
        } else {
            file_path = std::filesystem::absolute("application.uva");
        }

        if(argc > 2) {
            temp_file_path = std::filesystem::absolute(argv[2]);
        }
        else {
            temp_file_path = file_path;
        }

        if(!std::filesystem::exists(file_path)) {
            throw std::runtime_error("input file does not exist");
        }

        if(!std::filesystem::is_regular_file(file_path)) {
            throw std::runtime_error("input file is not a regular file");
        }

        std::string source = uva::file::read_all_text<char>(temp_file_path);

        uva::lang::lexer l(file_path.string(), source);

        // Note we are writing directly to the cout instead of saving and encoding the output

        std::cout << "{\n";
        std::cout << "\t\"tokens\": [";

        // for(const auto& token : l.tokens()) {
        //     switch(token.type()) {
        //         case uva::lang::lexer::token_type::token_keyword:
        //         case uva::lang::lexer::token_type::token_literal:
        //             std::cout << "\t";
        //             std::cout << "{ \"type\": \"keyword\", \"content\": \"";
        //             std::cout << token.content();
        //             std::cout << "\" }" << std::endl;
        //         break;
        //     }
        // }

        std::cout << "],\n";

        std::cout << "\t\"declarations\": [";

        uva::lang::parser p;
        uva::lang::parser::ast_node root_node = p.parse_all(l);

        size_t node_i = 0;

        for(const auto& node : root_node.childrens()) {
            
            if(node.type() == uva::lang::parser::ast_node_type::ast_node_classdecl) {
                if(node_i) {
                    std::cout << ",";
                }

                node_i++;

                std::cout << "\n";

                const uva::lang::parser::ast_node* decname_node = node.child_from_type(uva::lang::parser::ast_node_type::ast_node_declname);
                const uva::lang::lexer::token& decname_token = decname_node->token();

                std::cout << "\t\t{\n\t\t\t\"type\": \"class\",\n\t\t\t\"name\": \"";
                std::cout << decname_token.content();
                std::cout << "\",\n\t\t\t\"location\": {\n\t\t\t\t\"file\": \"";
                std::cout << decname_token.m_file_name;
                std::cout << "\",\n";
                std::cout << "\t\t\t\t\"line\": ";
                std::cout << decname_token.start.line;
                std::cout << ",\n\t\t\t\t\"column\": ";
                std::cout << decname_token.start.column;
                std::cout << ",\n\t\t\t\t\"offset\": ";
                std::cout << decname_token.start.offset;
                std::cout << "\n\t\t\t}";
                std::cout << ",\n\t\t\t\"references\": [";

                size_t token_i = 0;

                for(const auto& token : l.tokens()) {
                    if(token.type() == uva::lang::lexer::token_type::token_identifier) {
                        if(token.content() == decname_token.content()) {
                            if(token_i) {
                                std::cout << ",";
                            }

                            token_i++;

                            std::cout << "\n\t\t\t\t{\n\t\t\t\t\t\"file\": \"";
                            std::cout << token.m_file_name;
                            std::cout << "\",\n\t\t\t\t\t\"line\": ";
                            std::cout << token.start.line;
                            std::cout << ",\n\t\t\t\t\t\"column\": ";
                            std::cout << token.start.column;
                            std::cout << ",\n\t\t\t\t\t\"offset\": ";
                            std::cout << token.start.offset;
                            std::cout << "\n\t\t\t\t}";
                        }
                    }
                }

                std::cout << "\n\t\t\t]";
                std::cout << "\n\t\t}";
            }
        }

        std::cout << "\n\t],\n";

        auto end = std::chrono::high_resolution_clock::now();

        std::cout << "\t\"elapsed\": \"" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\"\n";

        std::cout << "}";

    } catch (const std::exception& e) {
        uva::console::log_error(e.what());
        return false;
    }

    return 0;
}