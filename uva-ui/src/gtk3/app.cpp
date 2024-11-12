// Include gtk
#include <gtk/gtk.h>

#include <uva-ui/app.hpp>

GtkApplication *gtkapp = nullptr;
uva::lang::ui::app* uvaapp = nullptr;

int argc = 0;
char** argv = nullptr;

void on_activate (GtkApplication* app, gpointer user_data) {
    uvaapp = reinterpret_cast<uva::lang::ui::app*>(user_data);
    uvaapp->on_init(argc, argv);
}

uva::lang::ui::app::app(std::string_view __name, std::string_view vendor)
{
    gtkapp = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (gtkapp, "activate", G_CALLBACK (on_activate), this);
}

uva::lang::ui::app::~app()
{
    set_theme(nullptr);
}

int uva::lang::ui::app::run(int __argc, char** __argv)
{
    argc = __argc;
    argv = __argv;

    int status = g_application_run (G_APPLICATION (gtkapp), 0, nullptr);

    g_object_unref (gtkapp);

    return 0;
}