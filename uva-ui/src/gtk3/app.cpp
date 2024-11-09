// Include gtk
#include <gtk/gtk.h>

#include <uva-ui/app.hpp>

GtkApplication *gtkapp = nullptr;

void on_activate (GtkApplication* app, gpointer user_data) {
    uva::lang::ui::app* uva_app = reinterpret_cast<uva::lang::ui::app*>(user_data);
    uva_app->on_init();
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

int uva::lang::ui::app::run()
{
    int argc = 0;
    char** argv = nullptr;

    int status = g_application_run (G_APPLICATION (gtkapp), argc, argv);

    g_object_unref (gtkapp);

    return 0;
}