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

std::shared_ptr<uva::lang::structure> application_class;
std::shared_ptr<uva::lang::object> application;

std::vector<uva::lang::extension*> extensions;

bool wxUvaApp::OnInit()
{
    try {
        vm_instance = std::make_shared<uva::lang::vm>();

        std::filesystem::path file_path;

        if(wxApp::argc > 1) {
            file_path = std::filesystem::absolute(wxApp::argv[1].ToStdString(wxMBConvUTF8()));
        } else {
            file_path = std::filesystem::absolute("application.uva");
        }

        application_class = p.parse(file_path, vm_instance);

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