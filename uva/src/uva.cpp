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

#ifdef __UVA_DEBUG__
    #define try if(true)
    #define catch(e) if(false)

    std::exception e;
#endif

int main(int argc, char** argv) {
    try {
        std::filesystem::path uva_executable_path = std::filesystem::absolute(argv[0]);
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

        // If the first instruction is a boot instruction, the execution is stopped and a new vm is launched
        // Loads a token untill the first non comment token

        uva::lang::lexer::token t = l.next_token();
        while(t.type() == uva::lang::lexer::token_type::token_comment) {
            t = l.next_token();
        }

        if(t.type() == uva::lang::lexer::token_type::token_keyword && t.content() == "boot") {
            t = l.next_token();

            if(t.content() != "(") {
                t.throw_error_at_current_position("Expected '(' after boot");
                return 1;
            }

            t = l.next_token();

            if(t.kind() != uva::lang::lexer::token_kind::token_string) {
                t.throw_error_at_current_position("Expected string after boot");
                return 1;
            }

            std::filesystem::path boot_path = uva_executable_path;
            boot_path.replace_filename(t.content());

            if(!std::filesystem::exists(boot_path)) {
                throw std::runtime_error("Boot executable does not exist. Expected to find it " + boot_path.string());
            }

            std::string boot_command = boot_path.string();

            for(int i = 1; i < argc; i++) {
                boot_command += " ";
                boot_command += argv[i];
            }

            return system(boot_command.c_str());
        }

        l.reset();

        uva::lang::parser p;
        uva::lang::parser::ast_node root_node = p.parse_all(l);

        uva::lang::interpreter interpreter;
        std::shared_ptr<uva::lang::object> tmp;
        interpreter.execute_all(root_node, tmp);

        auto application_class = interpreter.find_class("Application");

        if(!application_class) {
            throw std::runtime_error("Application class not found");
        }

        auto run_it = application_class->methods.find("run");

        if(run_it == application_class->methods.end()) {
            throw std::runtime_error("run function not defined in class Application. Define it so uva know where to start the application");
        }
        
        application = std::make_shared<uva::lang::object>(application_class);

        std::shared_ptr<uva::lang::object> ret = interpreter.call(application_class, application, run_it->second, {});

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