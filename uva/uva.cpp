#include <filesystem>

#include <parser/parser.hpp>
#include <lexer/lexer.hpp>
//#include <vm/vm.hpp>

#include <console.hpp>
#include <uva/file.hpp>
#include <interpreter/interpreter.hpp>
#include <extension/extension.hpp>

using namespace uva;

std::shared_ptr<uva::lang::object> application;
std::vector<uva::lang::extension*> extensions;

int main(int argc, char** argv) {
    try {
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

        uva::lang::parser p;
        uva::lang::parser::ast_node root_node = p.parse_all(l);

        uva::lang::interpreter interpreter;
        interpreter.execute_all(root_node);

        auto application_class = interpreter.find_class("Application");

        if(!application_class) {
            throw std::runtime_error("Application class not found");
        }

        auto run_it = application_class->methods.find("run");

        if(run_it == application_class->methods.end()) {
            throw std::runtime_error("run method not defined in class Application. Define it so uva know where to start the application");
        }
        
        application = std::make_shared<uva::lang::object>(application_class);

        std::shared_ptr<uva::lang::object> ret = interpreter.call(application, run_it->second, {});

        if(!ret) {
            return 0;
        }

        if(ret) {
            // TODO: Treat the return value
            int ret_value = *ret->as<int>();
            return ret_value;
        }
    } catch (const std::exception& e) {
        uva::console::log_error(e.what());
        return false;
    }

    return 0;
}