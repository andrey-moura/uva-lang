#include <uva/drawing.hpp>

#include <gtk/gtk.h>

struct window_data {
    GtkWidget* window = nullptr;
};

uva::drawing::window::window(std::string_view __title)
{
    GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title (GTK_WINDOW (window), __title.data());

    window_data data;
    data.window = window;

    os_specific_data_as<window_data>() = data;

    // Setup the drawing area

    GtkWidget* drawing_area = gtk_drawing_area_new();

    gtk_container_add(GTK_CONTAINER(window), drawing_area);
}

void uva::drawing::window::show(bool maximized)
{
    GtkWidget* window = os_specific_data_as<window_data>().window;

    if(maximized) {
        gtk_window_maximize(GTK_WINDOW(window));
    }

    gtk_widget_show_all(window);
}