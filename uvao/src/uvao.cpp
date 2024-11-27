#include <filesystem>

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
        std::filesystem::path uva_executable_path = argv[0];
        //vm_instance = std::make_shared<uva::lang::vm>();

        std::filesystem::path file_path;

        if(argc > 1) {
            file_path = std::filesystem::absolute(argv[1]);
        } else {
            file_path = std::filesystem::absolute("application.uva");
        }

        if(!std::filesystem::exists(file_path)) {
            throw std::runtime_error("input file does not exist");
        }

        if(!std::filesystem::is_regular_file(file_path)) {
            throw std::runtime_error("input file is not a regular file");
        }

        std::string source = uva::file::read_all_text<char>(file_path);

        uva::lang::lexer l(file_path.string(), source);

        // Copy the string since it can be moved by the parser
        std::vector<std::string> string_table;

        for(const auto& token : l.tokens()) {
            switch(token.type()) {
                case uva::lang::lexer::token_type::token_identifier:
                    string_table.push_back(token.content());
                break;
                case uva::lang::lexer::token_type::token_literal:
                    if(token.kind() == uva::lang::lexer::token_kind::token_string) {
                        string_table.push_back(token.content());
                    }
                break;
            }
        }

        std::sort(string_table.begin(), string_table.end());

        std::filesystem::path object_path = std::filesystem::absolute(file_path.stem());
        object_path.replace_extension(".uvao");

        std::ofstream object_stream(object_path, std::ios::binary);

        if(!object_stream.is_open()) {
            throw std::runtime_error("unable to create object file");
        }

        for(const auto& str : string_table) {
            object_stream.write(str.data(), str.size());
            object_stream.put(0);
        }

        uva::lang::parser p;
        uva::lang::parser::ast_node root_node = p.parse_all(l);

        for(const auto& node : root_node.childrens()) {
            object_stream.put((char)node.type());

            switch(node.type())
            {
            case uva::lang::parser::ast_node_type::ast_node_classdecl: {
                std::string_view class_name = node.child_content_from_type(uva::lang::parser::ast_node_type::ast_node_declname);
                auto it = std::lower_bound(string_table.begin(), string_table.end(), class_name);

                if(it == string_table.end() || *it != node.child_content_from_type(uva::lang::parser::ast_node_type::ast_node_declname)) {
                    throw std::runtime_error("class name not found in string table");
                }

                uint32_t index = std::distance(string_table.begin(), it);

                object_stream.write((char*)&index, sizeof(index));
            }
            break;
            default:
                throw std::runtime_error("unexpected node type: " + std::to_string((int)node.type()));
            break;
            }
        }

    } catch (const std::exception& e) {
        uva::console::log_error(e.what());
        return false;
    }

    return 0;
}