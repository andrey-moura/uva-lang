#include <extension/extension.hpp>
#include <iostream>

#include <wx/wx.h>

#include <uva/string.hpp>
#include <vm/vm.hpp>

class uva_wx_extension : public uva::lang::extension {
public:
    uva_wx_extension() : extension("uva-wx") {
    }
public:
    virtual void init() override {
        // This is called when we're read to load resources.
        // This extension does not have any resources to load.
    }

    virtual void load_in_vm(uva::lang::vm* vm) override {
        // This is called when the VM is ready to load classes and objects.
        
        std::shared_ptr<uva::lang::structure> appClass = std::make_shared<uva::lang::structure>("wx::App");
        std::shared_ptr<uva::lang::structure> frameClass = std::make_shared<uva::lang::structure>("wx::Frame");

        vm->load(appClass);
        vm->load(frameClass);

        frameClass->methods["new"] = uva::lang::method("new", uva::lang::method_storage_type::instance_method, {}, [](uva::lang::object* object, const var& params) {
            std::string title;
            if(params) {
                if(params["title"]) {
                    title = params["title"].to_s();
                }
            }
            wxFrame* frame = new wxFrame(nullptr, wxID_ANY, title);
            object->native = frame;
            if(object->derived_instance) {
                // std::cout << "new instance of wxFrame's derived: " << object->derived_instance->cls->name << std::endl;
                // auto view_path = std::filesystem::absolute("views") / uva::string::to_snake_case( object->derived_instance->cls->name );
                // view_path.replace_extension("wxvhtml");

                // std::cout << "searching for view at: " << view_path << std::endl;

                // if(std::filesystem::exists(view_path)) {
                //     std::cout << "view exists" << std::endl;
                // }
            }
            return nullptr;
        });

        frameClass->methods["show"] = uva::lang::method("show", uva::lang::method_storage_type::instance_method, {}, [](uva::lang::object* object, const var& params) {
            wxFrame* frame = (wxFrame*)(object->native);
            frame->Show();
            return nullptr;
        });
    }
};

static uva_wx_extension wx_extension = uva_wx_extension();