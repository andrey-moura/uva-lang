//This file was generated with uva-languanizer. Modifications to this file may be overridden.
//This file has no licence, simply use it as you need.
//Generated at: Thu, 17 Oct 24 01:49:18 +0000.

std::shared_ptr<Class> wxClass = Class::create("wx", {

	Method("wx_message_box", method_storage_type::instance_method, { "message", "caption", "style", "parent", "x", "y" }, [](Object* object, const var& params) {
		wxString message = wx_string_from_var(params["message"]);
		wxString caption = wx_string_from_var(params["caption"]);
		long style = params["style"].as<var::integer>();
		wxWindow* parent = nullptr;
		int x = wxDefaultCoord;
		int y = wxDefaultCoord;

		wx* native_object = (wx*)(object);
		native_object->wxMessageBox(message, caption, style, parent, x, y);
		return nullptr;
	})

});