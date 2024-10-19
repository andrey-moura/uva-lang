#include <filesystem>

#include <parser/parser.hpp>
#include <vm/vm.hpp>

#include <console.hpp>
#include <uva/file.hpp>
#include <extension/extension.hpp>

#include <wx/wx.h>

using namespace uva;

std::shared_ptr<uva::lang::vm> vm_instance;

class wxUvaApp : public wxApp
{
private:
    parser p;
public:
	wxUvaApp() = default;
	~wxUvaApp() = default;

	virtual bool OnInit();
};

wxIMPLEMENT_APP(wxUvaApp);

std::shared_ptr<uva::lang::Class> application_class;
std::shared_ptr<uva::lang::object> application;

std::vector<uva::lang::extension*> extensions;

bool wxUvaApp::OnInit()
{
    try {
        vm_instance = std::make_shared<uva::lang::vm>();

        //Class* application_class = p.parse(std::filesystem::absolute("application.uva"));
        application_class = p.parse(std::filesystem::path("/home/andrey/Moonslate/teste/application.uva"));
        vm_instance->load(application_class);

        auto it = application_class->methods.find("init");

        if(it == application_class->methods.end()) {
            throw std::runtime_error("init method not defined in class Application");
        }
        
        application = std::make_shared<uva::lang::object>(application_class);

        std::shared_ptr<uva::lang::object> ret = vm_instance->call(application, it->second);

        if(!ret || ret->cls != vm_instance->True) {
            std::cout << "init not returned true. Exiting..." << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        uva::console::log_error(e.what());
        return false;
    }

	return true;
}

// int main(int argc, char** argv) {
//     if(argc > 1) {
//         std::string_view arg = argv[1];

//         if(arg == "new") {
//             std::string_view project_name = argv[2];

//             std::filesystem::path project_folder = std::filesystem::absolute(project_name);

//             if(std::filesystem::exists(project_folder)) {
//             uva::console::log_error("Project folder already exists");
//             }

//             std::filesystem::create_directory(project_folder);

//             std::string application = R"~~(
//     class Application do
//         def run do
//             puts "Hello World"
//         end
//     end
//             )~~";

//             uva::file::write_all_text(project_folder / "application.uva", application);

//             uva::console::log_success("Project created");
//             return 0;
//         } else if(arg == "build") {
//             std::filesystem::path project_folder = std::filesystem::absolute("build");

//             if(!std::filesystem::exists(project_folder)) {
//                 std::filesystem::create_directory(project_folder);
//             }

//             uva::console::log_success("Project built");
//             return 0;
//         }
//     } else {
//         for(auto& extension : extension::extensions) {
//             extension->init();
//         }

//         parser p;
//         Class* application_class = p.parse(std::filesystem::absolute("application.uva"));

//         auto it = application_class->methods.find("run");

//         if(it == application_class->methods.end()) {
//             throw std::runtime_error("run method not defined in class Application");
//         }

//         Object application(application_class);

//         application.call(it->second);
//     }

//     return 0;
// }