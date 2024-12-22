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

        //std::filesystem::current_path(file_path.parent_path());

        std::string source = uva::file::read_all_text<char>(file_path);

        uva::lang::lexer l(file_path.string(), source);

        uva::lang::preprocessor preprocessor;
        preprocessor.process(file_path.string(), l);

        uva::lang::parser p;
        uva::lang::parser::ast_node root_node = p.parse_all(l);

        uva::lang::interpreter interpreter;
        std::shared_ptr<uva::lang::object> tmp;
        std::shared_ptr<uva::lang::object> ret = interpreter.execute_all(root_node, tmp);

        interpreter.start_extensions();

        if(!ret) {
            return 0;
        }

        if(ret) {
            // TODO: Treat the return value
            int ret_value = ret->as<int>();
            return ret_value;
        }
    } catch (const std::exception& e) {
        uva::console::log_error(e.what());
        return false;
    }

    return 0;
}