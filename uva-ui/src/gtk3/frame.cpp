#include <frame.hpp>

#include <uva-ui/app.hpp>

#include <gtk/gtk.h>

extern GtkApplication *gtkapp;
extern uva::lang::ui::app* uvaapp;

struct window_data {
    GtkWidget* window;
};

static gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    //auto attributes = uvaapp->theme()->window();
    std::string color_rgb = "red";
    guint width, height;
    GdkRGBA color;
    gdk_rgba_parse(&color, color_rgb.c_str());

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgb(cr, color.red, color.green, color.blue);

    cairo_rectangle(cr, 0, 0, width, height);

    cairo_fill(cr);

    return FALSE;
}

uva::lang::ui::frame::frame(std::string_view __title)
{
    GtkWidget* window = gtk_application_window_new (gtkapp);

    gtk_window_set_title (GTK_WINDOW (window), __title.data());
    gtk_window_set_default_size (GTK_WINDOW (window), 400, 400);

    os_specific_data_as<window_data>()->window = window;

    // Setup the drawing area

    GtkWidget* drawing_area = gtk_drawing_area_new();

    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    g_signal_connect (drawing_area, "draw", G_CALLBACK (draw_callback), NULL);
}

uva::lang::ui::frame::~frame()
{

}

void uva::lang::ui::frame::show(bool maximized)
{
    GtkWidget* window = os_specific_data_as<window_data>()->window;

    if(maximized) {
        gtk_window_maximize(GTK_WINDOW(window));
    }

    gtk_widget_show_all(window);
}

void uva::lang::ui::frame::hide()
{

}