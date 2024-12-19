#include <frame_class.hpp>

#include <frame.hpp>

#include <object.hpp>
#include <class.hpp>

class uvalang_ui_frame : public uva::lang::ui::frame
{
public:
    uvalang_ui_frame(std::shared_ptr<uva::lang::object> __object, std::string_view title)
        : uva::lang::ui::frame(title), object(__object)
    {

    }
protected:
    std::shared_ptr<uva::lang::object> object;
};

std::shared_ptr<uva::lang::structure> uva::lang::frame_class::create(uva::lang::interpreter* interpreter)
{
    auto ui_frame_class = std::make_shared<uva::lang::structure>("UI.Frame");

    ui_frame_class->methods = {
        { "new", uva::lang::method("new", uva::lang::method_storage_type::instance_method, {"title"}, [](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params){
            std::string title = params[0]->as<std::string>();

            uvalang_ui_frame* frame = new uvalang_ui_frame(object, title);

            object->set_native_ptr(frame);

            return nullptr;
        })},
        { "show", uva::lang::method("show", uva::lang::method_storage_type::instance_method, {"maximized"}, [](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params){
            uva::lang::object* maximized = params[0].get();
            uvalang_ui_frame& frame = object->as<uvalang_ui_frame>();
            frame.show(maximized && maximized->is_present());

            return nullptr;
        })},
        { "hide", uva::lang::method("hide", uva::lang::method_storage_type::instance_method, {}, [](std::shared_ptr<uva::lang::object> object, std::vector<std::shared_ptr<uva::lang::object>> params){
            uvalang_ui_frame& frame = object->as<uvalang_ui_frame>();
            frame.hide();

            return nullptr;
        })},
    };

    return ui_frame_class;
}