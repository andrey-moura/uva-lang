#include <iostream>
#include <filesystem>

#include <andy/lang/parser.hpp>
#include <andy/lang/lexer.hpp>
//#include <vm/vm.hpp>
#include <uva/console.hpp>
#include <uva/file.hpp>
#include <andy/lang/interpreter.hpp>
#include <andy/lang/extension.hpp>
#include <andy/lang/preprocessor.hpp>
#include <andy/lang/config.hpp>

using namespace andy;

std::shared_ptr<andy::lang::object> application;

#ifdef __UVA_DEBUG__
    #define try if(true)
    #define catch(e) if(false)

    std::exception e;
#endif

int main(int argc, char** argv) {
    try {
        std::filesystem::path andy_executable_path = argv[0];
        //vm_instance = std::make_shared<andy::lang::vm>();

        std::filesystem::path file_path;

        if(argc > 1) {
            std::string_view arg = argv[1];

            if(arg.starts_with("--")) {
                if(arg == "--help") {
                    std::cout << "Usage: " << argv[0] << " [file]" << std::endl;
                    std::cout << std::endl;
                    std::cout << "Options: " << std::endl;
                    uva::console::print_warning("  --help");
                    std::cout << "     Display this information" << std::endl;
                    uva::console::print_warning("  --version");
                    std::cout << "  Display the version of the andy language" << std::endl;
                    return 0;
                } else if(arg == "--version") {
                    std::cout << ANDYLANG_VERSION << std::endl;
                    return 0;
                } else {
                    arg.remove_prefix(2);
                    file_path = andy::lang::config::src_dir() / "utility" / arg;
                    file_path.replace_extension(".andy");

                    if(!std::filesystem::exists(file_path)) {
                        throw std::runtime_error("utility does not exist");
                    }

                    if(!std::filesystem::is_regular_file(file_path)) {
                        throw std::runtime_error("utility is not a regular file");
                    }
                }
            }
        }

        if(file_path.empty()) {
            if(argc > 1) {
                file_path = std::filesystem::absolute(argv[1]);
            } else {
                file_path = std::filesystem::absolute("application.andy");
            }

            if(!std::filesystem::exists(file_path)) {
                throw std::runtime_error("input file does not exist");
            }

            if(!std::filesystem::is_regular_file(file_path)) {
                throw std::runtime_error("input file is not a regular file");
            }
        }

        //std::filesystem::current_path(file_path.parent_path());

        std::string source = uva::file::read_all_text<char>(file_path);

        andy::lang::lexer l(file_path.string(), source);

        andy::lang::preprocessor preprocessor;
        preprocessor.process(file_path.string(), l);

        andy::lang::parser p;
        andy::lang::parser::ast_node root_node = p.parse_all(l);

        andy::lang::interpreter interpreter;
        std::shared_ptr<andy::lang::object> tmp;
        std::shared_ptr<andy::lang::object> ret = interpreter.execute_all(root_node, tmp);

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