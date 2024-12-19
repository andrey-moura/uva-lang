#include <lang/lang.hpp>

#include <interpreter/interpreter.hpp>

#include <app_class.hpp>
#include <app.hpp>

class uvalang_ui_app : public uva::lang::ui::app
{
public:
    uvalang_ui_app(uva::lang::interpreter* __interpreter, std::shared_ptr<uva::lang::object> __application_instance)
        : uva::lang::ui::app("uva", "uva"), interpreter(__interpreter)
    {
        application_instance = __application_instance;
    }
protected:
    uva::lang::interpreter* interpreter = nullptr;
    std::shared_ptr<uva::lang::object> application_instance;
public:
    virtual void on_init(int argc, char** argv) override
    {
        auto run_it = application_instance->cls->methods.find("run");

        if(run_it == application_instance->cls->methods.end()) {
            throw std::runtime_error("run method not defined in class Application");
        }

        interpreter->call(application_instance->cls, application_instance, run_it->second, {});
    }
};

std::shared_ptr<uva::lang::structure> uva::lang::app_class::create(uva::lang::interpreter* interpreter)
{
    auto AppClass = std::make_shared<uva::lang::structure>("UI.Application");

    AppClass->methods = {
        { "new", uva::lang::method("new", uva::lang::method_storage_type::instance_method, {}, [interpreter](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params){
            uvalang_ui_app* app = new uvalang_ui_app(interpreter, object->derived_instance);
            object->set_native_ptr(app);

            app->run(0, nullptr);

            return nullptr;
        })},
    };

    return AppClass;
}