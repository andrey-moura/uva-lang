#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <uva/var.hpp>
#include <uva/file.hpp>
#include <console.hpp>

#include <uva-ui/app.hpp>
#include <uva-ui/frame.hpp>
#include <uva-ui/theme.hpp>
#include <uva-ui/style.hpp>

#include <preprocessor/preprocessor.hpp>
#include <extension/extension.hpp>
#include <interpreter/interpreter.hpp>
#include <lang/lang.hpp>

std::vector<uva::lang::extension*> extensions;
uva::lang::interpreter interpreter;

var call_and_convert_to_dictionary(std::shared_ptr<uva::lang::object> object, std::string_view method_name)
{
    auto it = object->cls->methods.find(std::string(method_name));

    if(it == object->cls->methods.end()) {
        return null;
    }

    std::shared_ptr<uva::lang::object> result = interpreter.call(object->cls, object, it->second, {});

    if(result->cls != interpreter.DictionaryClass) {
        return null;
    }

    return result->to_var();
}

class uvalang_ui_theme : public uva::lang::ui::theme
{
public:
    uvalang_ui_theme(std::shared_ptr<uva::lang::object> __object)
        : uva::lang::ui::theme(), object(__object)
    {

    }
protected:
    std::shared_ptr<uva::lang::object> object;
public:
    virtual var request(std::string_view what) override
    {
        return call_and_convert_to_dictionary(object, what);
    }
};

class uvalang_ui_frame : public uva::lang::ui::frame
{
public:
    uvalang_ui_frame(std::shared_ptr<uva::lang::object> __object, std::string_view title)
        : uva::lang::ui::frame(title), object(__object)
    {

    }
protected:
    std::shared_ptr<uva::lang::object> object;
public:
    virtual var render() override
    {
        return call_and_convert_to_dictionary(object, "render");
    }
};

class uvalang_ui_style : public uva::lang::ui::style
{
public:
    uvalang_ui_style(std::shared_ptr<uva::lang::object> __object)
        : uva::lang::ui::style(), object(__object)
    {

    }
public:
    virtual var request(std::string_view what) override
    {
        return call_and_convert_to_dictionary(object, what);
    }
protected:
    std::shared_ptr<uva::lang::object> object;
};

class uvalang_ui_app : public uva::lang::ui::app
{
protected:
    std::shared_ptr<uva::lang::structure> ui_application_class = std::make_shared<uva::lang::structure>("UI.Application");
    std::shared_ptr<uva::lang::structure> ui_window_class = std::make_shared<uva::lang::structure>("UI.Frame");
public:
    uvalang_ui_app()
        : uva::lang::ui::app("uva", "uva")
    {

    }

    virtual void on_init(int argc, char** argv) override
    {
        std::filesystem::path uva_executable_path = argv[0];

        ui_application_class->methods = {
            { "new", uva::lang::method("new", uva::lang::method_storage_type::instance_method, {}, [this](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params){
                uvalang_ui_app* app = this;
                object->set_native_ptr(app);

                return nullptr;
            })},
            { "set_theme", uva::lang::method("set_theme", uva::lang::method_storage_type::instance_method, {"theme"}, [this](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params){
                std::shared_ptr<uva::lang::object> theme_object = params[0];

                if(!theme_object->cls->base || theme_object->cls->base->name != "UI.Theme") {
                    throw std::runtime_error("theme must have the UI.Theme base class");
                }

                // The theme is now owned by the application
                uvalang_ui_theme* theme_native = theme_object->move_native_ptr<uvalang_ui_theme>();

                uva::lang::ui::app* app = &object->base_instance->as<uva::lang::ui::app>();

                app->set_theme(theme_native);

                return nullptr;
            })},
            { "set_style", uva::lang::method("set_style", uva::lang::method_storage_type::instance_method, {"style"}, [this](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params){
                std::shared_ptr<uva::lang::object> style_object = params[0];

                if(!style_object->cls->base || style_object->cls->base->name != "UI.Style") {
                    throw std::runtime_error("style must have the UI.Style base class");
                }

                // The style is now owned by the application
                uvalang_ui_style* style_native = style_object->move_native_ptr<uvalang_ui_style>();

                uva::lang::ui::app* app = &object->base_instance->as<uva::lang::ui::app>();

                app->set_style(style_native);

                return nullptr;
            })},
        };

        ui_window_class->methods = {
            { "new", uva::lang::method("new", uva::lang::method_storage_type::instance_method, {"title"}, [this](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params){
                std::string title = params[0]->as<std::string>();

                uvalang_ui_frame* frame = new uvalang_ui_frame(object, title);

                object->set_native_ptr(frame);

                return nullptr;
            })},
            { "show", uva::lang::method("show", uva::lang::method_storage_type::instance_method, {"maximized"}, [this](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params){
                uva::lang::object* maximized = params[0].get();
                uvalang_ui_frame& frame = object->as<uvalang_ui_frame>();
                frame.show(maximized && maximized->is_present());

                return nullptr;
            })},
            { "hide", uva::lang::method("hide", uva::lang::method_storage_type::instance_method, {}, [this](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params){
                uvalang_ui_frame& frame = object->as<uvalang_ui_frame>();
                frame.hide();

                return nullptr;
            })},
        };

        std::shared_ptr<uva::lang::structure> ui_theme_class = std::make_shared<uva::lang::structure>("UI.Theme");
        ui_theme_class->methods = {
            { "new", uva::lang::method("new", uva::lang::method_storage_type::instance_method, {}, [this](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params){
                uvalang_ui_theme* theme = new uvalang_ui_theme(object);
                object->set_native_ptr(theme);

                return nullptr;
            })},
        };

        std::shared_ptr<uva::lang::structure> ui_style_class = std::make_shared<uva::lang::structure>("UI.Style");
        ui_style_class->methods = {
            { "new", uva::lang::method("new", uva::lang::method_storage_type::instance_method, {}, [this](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params){
                uvalang_ui_style* style = new uvalang_ui_style(object);
                object->set_native_ptr(style);

                return nullptr;
            })},
        };

        interpreter.load(ui_application_class);
        interpreter.load(ui_window_class);
        interpreter.load(ui_theme_class);
        interpreter.load(ui_style_class);

        std::filesystem::path file_path = std::filesystem::absolute("application.uva");

        if(!std::filesystem::exists(file_path)) {
            throw std::runtime_error("input file does not exist");
        }

        if(!std::filesystem::is_regular_file(file_path)) {
            throw std::runtime_error("input file is not a regular file");
        }

        std::string source = uva::file::read_all_text<char>(file_path);

        uva::lang::lexer l(file_path.string(), source);

        uva::lang::preprocessor preprocessor(uva_executable_path);
        preprocessor.process(file_path.string(), l);

        if(preprocessor.has_specified_vm()) {
            if(preprocessor.specified_vm() != uva_executable_path.stem()) {
                exit(preprocessor.launch_vm(argc, argv));
            }
        }

        uva::lang::parser p;

        uva::lang::parser::ast_node root_node = p.parse_all(l);

        std::shared_ptr<uva::lang::object> tmp;

        interpreter.execute_all(root_node, tmp);

        auto application_class = interpreter.find_class("Application");

        if(!application_class) {
            throw std::runtime_error("Application class not found");
        }

        if(!application_class->base || application_class->base->name != "UI.Application") {
            throw std::runtime_error("Application class must inherit from UI.Application");
        }

        if(!application_class) {
            throw std::runtime_error("Application class not found");
        }

        std::shared_ptr<uva::lang::object> application_instance = uva::lang::object::instantiate(&interpreter, application_class, nullptr);

        auto run_it = application_class->methods.find("run");

        if(run_it == application_class->methods.end()) {
            throw std::runtime_error("run method not defined in class Application. Define it so uva know where to start the application");
        }

        interpreter.call(application_class, application_instance, run_it->second, {});
    }
};

int main(int argc, char** argv) {
    try {
        uvalang_ui_app app;
        return app.run(argc, argv);
    } catch(const std::exception& e) {
        uva::console::log_error(e.what());
    }

    return 0;
}