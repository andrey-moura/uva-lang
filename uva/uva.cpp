#include <filesystem>

#include <parser/parser.hpp>
#include <vm/vm.hpp>

#include <console.hpp>
#include <uva/file.hpp>
#include <extension/extension.hpp>

using namespace uva;

std::shared_ptr<uva::lang::vm> vm_instance;

std::shared_ptr<uva::lang::structure> application_class;
std::shared_ptr<uva::lang::object> application;
parser p;
std::vector<uva::lang::extension*> extensions;

int main(int argc, char** argv) {
    try {
        vm_instance = std::make_shared<uva::lang::vm>();

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

        application_class = p.parse(file_path, vm_instance);

        auto it = application_class->methods.find("run");

        if(it == application_class->methods.end()) {
            throw std::runtime_error("run method not defined in class Application");
        }
        
        application = std::make_shared<uva::lang::object>(application_class);

        std::shared_ptr<uva::lang::object> ret = vm_instance->call(application, it->second);

        if(!ret) {
            return 0;
        }

        if(ret) {
            // TODO: Treat the return value
            return 0;
        }
    } catch (const std::exception& e) {
        uva::console::log_error(e.what());
        return false;
    }

    return 0;
}