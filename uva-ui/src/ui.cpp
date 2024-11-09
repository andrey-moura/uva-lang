#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <uva-ui/app.hpp>
#include <uva-ui/frame.hpp>
#include <uva-ui/theme.hpp>

#include <uva/file.hpp>
#include <console.hpp>

#include <extension/extension.hpp>
#include <interpreter/interpreter.hpp>
#include <lang/lang.hpp>

std::vector<uva::lang::extension*> extensions;
uva::lang::interpreter interpreter;

class uvalang_ui_theme : public uva::lang::ui::theme
{
public:
    uvalang_ui_theme(std::shared_ptr<uva::lang::object> object)
        : uva::lang::ui::theme()
    {

    }
protected:
    std::shared_ptr<uva::lang::object> object;
public:
    virtual std::map<std::string, std::string> window() override
    {
        std::cout << "window" << std::endl;

        auto window_it = object->cls->methods.find("window");

        if(window_it == object->cls->methods.end()) {
            throw std::runtime_error("window method not found in theme class");
        }

        std::shared_ptr<uva::lang::object> window_object = interpreter.call(object->cls, object, window_it->second, {});

        if(window_object->cls != interpreter.DictionaryClass) {
            throw std::runtime_error("window method must return a map");
        }

        std::map<std::string, std::string> window_map;

        for(auto& [key, value] : window_object->as<uva::lang::dictionary>()) {
            std::string key_str = interpreter.call(key->cls, key, interpreter.StringClass->methods["to_s"], {})->as<std::string>();
            std::string value_str = interpreter.call(key->cls, value, interpreter.StringClass->methods["to_s"], {})->as<std::string>();

            window_map[key_str] = value_str;
        }

        return window_map;
    }
};

class uvalang_ui_app : public uva::lang::ui::app
{
protected:
    std::shared_ptr<uva::lang::structure> ui_application_class = std::make_shared<uva::lang::structure>("UI.Application");
    std::shared_ptr<uva::lang::structure> ui_window_class = std::make_shared<uva::lang::structure>("UI.Frame");
public:
    uvalang_ui_app(std::string_view __name, std::string_view vendor)
        : uva::lang::ui::app(__name, vendor)
    {

    }

    virtual void on_init() override
    {
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

                uvalang_ui_app* app = &object->as<uvalang_ui_app>();

                app->set_theme(theme_native);

                return nullptr;
            })},
        };

        ui_window_class->methods = {
            { "new", uva::lang::method("new", uva::lang::method_storage_type::instance_method, {"title"}, [this](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params){
                std::string title = params[0]->as<std::string>();

                uva::lang::ui::frame* frame = new uva::lang::ui::frame(title);

                object->set_native_ptr(frame);

                return nullptr;
            })},
            { "show", uva::lang::method("show", uva::lang::method_storage_type::instance_method, {"maximized"}, [this](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params){
                uva::lang::object* maximized = params[0].get();
                uva::lang::ui::frame& frame = object->as<uva::lang::ui::frame>();
                frame.show(maximized && maximized->is_present());

                return nullptr;
            })},
            { "hide", uva::lang::method("hide", uva::lang::method_storage_type::instance_method, {}, [this](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params){
                uva::lang::ui::frame& frame = object->as<uva::lang::ui::frame>();
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

        interpreter.load(ui_application_class);
        interpreter.load(ui_window_class);
        interpreter.load(ui_theme_class);

        std::filesystem::path file_path = std::filesystem::absolute("application.uva");

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

#ifndef _NDEBUG
    #define try if(true)
    #define catch(e) if(false)

    std::exception e;
#endif

int main(int argc, char** argv) {
    try {
        uvalang_ui_app app("uva-ui", "uva");
        return app.run();
    } catch(const std::exception& e) {
        uva::console::log_error(e.what());
    }

    return 0;
}