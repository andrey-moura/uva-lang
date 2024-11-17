#include <frame.hpp>

#include <uva-ui/app.hpp>

#include <gtk/gtk.h>

extern GtkApplication *gtkapp;
extern uva::lang::ui::app* uvaapp;

cairo_t *current_cr = nullptr;

struct window_data {
    GtkWidget* window;
};

struct view_element {
    // Class list of the elment. It is sorted
    view_element(var::dictionary_type layout)
    {
        if(layout["class"]) {
            std::vector<std::string> stdclasses;
            var::array_type classes = layout["class"].as<var::array>();

            for(auto& cls : classes) {
                stdclasses.push_back(cls.as<var::string>());
            }

            set_classes(stdclasses);
        }

        if(layout["childrens"]) {
            var::array_type childrens = layout["childrens"].as<var::array>();

            for(auto& child : childrens) {
                if(child.is_a<var::dictionary>()) {
                    view_element child_element(child.as<var::dictionary>());
                    this->childrens.push_back(child_element);
                }
            }
        }

        if(layout["text"]) {
            text = layout["text"].as<var::string>();
        }
    }

    std::vector<std::string> classes;
    std::vector<view_element> childrens;
    std::string text;

    int x = 0;
    int y = 0;

    int width = 0;
    int height = 0;

    bool has_class(const std::string& cls) const {
        return std::binary_search(classes.begin(), classes.end(), cls);
    }

    bool add_class(const std::string& cls) {
        if(!has_class(cls)) {
            classes.push_back(cls);
            std::sort(classes.begin(), classes.end());
            return true;
        }

        return false;
    }

    void set_classes(const std::vector<std::string>& cls) {
        classes = cls;
        std::sort(classes.begin(), classes.end());
    }

    // Função auxiliar para calcular o tamanho do texto
    std::pair<int, int> calc_text_extent(const std::string& text) {
        cairo_text_extents_t extents;
        cairo_text_extents(current_cr, text.c_str(), &extents);

        return { extents.width, extents.height };
    }

    int best_height() {
        if(has_class("flex-spacer")) {
            return -1;
        } else if(text.size()) {
            auto [w, h] = calc_text_extent(text);
            return h;
        } else {
            int total_height = 0;

            for(auto& child : childrens) {
                int child_best_height = child.best_height();

                if(child_best_height == -1) {
                    child_best_height = 0;
                }

                total_height += child_best_height;
            }

            return total_height;
        }

        return 0;
    }

    int best_width() {
        if(has_class("flex-spacer")) {
            return -1;
        } else if(text.size()) {
            auto [w, h] = calc_text_extent(text);
            return w;
        } else {
            int total_width = 0;

            for(auto& child : childrens) {
                int child_best_width = child.best_width();

                if(child_best_width == -1) {
                    child_best_width = 0;
                }

                total_width += child_best_width;
            }

            return total_width;
        }

        return 0;
    }

    // Método para calcular atributos de layout (x, y, width, height) recursivamente
    void calculate_layout(int parent_x = 0, int parent_y = 0) {
        x = parent_x;
        y = parent_y;

        int total_spacers = 0;
        int available_height = height;
        int available_width = width;

        for(auto& child : childrens) {
            int child_best_width = child.best_width();
            int child_best_height = child.best_height();

            if(has_class("flex-vertical")) {
                if(child_best_height == -1) {
                    total_spacers++;
                } else {
                    available_height -= child_best_height;
                }
            } else {
                if(child_best_width == -1) {
                    total_spacers++;
                } else {
                    available_width -= child_best_width;
                }
            }
        }

        int child_x = x;
        int child_y = y;

        for(auto& child : childrens) {
            int child_best_width = child.best_width();
            int child_best_height = child.best_height();

            if(child_best_height == -1) {
                if(has_class("flex-vertical")) {
                    child_best_height = available_height / total_spacers;
                }
            }

            if(child_best_width == -1) {
                if(has_class("flex-horizontal")) {
                    child_best_width = available_width / total_spacers;
                }
            }

            child.width = child_best_width;
            child.height = child_best_height;

            if(has_class("flex-horizontal")) {
                child.height = height;
            } else {
                child.width = width;
            }

            child.calculate_layout(child_x, child_y);

            if(has_class("flex-horizontal")) {
                child_x += child_best_width;
            } else {
                child_y += child_best_height;
            }
        }
    }
};

void draw_element(GtkWidget *widget, cairo_t *cr, view_element& element, var::dictionary_type theme)
{
    if(element.text.size()) {
        var style_var = uvaapp->style()->request(element.classes.back());

        if(style_var.is_a<var::dictionary>()) {
            var font_size = style_var.as<var::dictionary>()["font-size"];

            if(font_size.is_a<var::integer>()) {
                int size = font_size.as<var::integer>();
                cairo_set_font_size(cr, size);
            }
        }

        GdkRGBA color;
        cairo_text_extents_t extents;
        cairo_text_extents(cr, element.text.c_str(), &extents);

        std::string background_color = theme["text"].as<var::string>();
        gdk_rgba_parse(&color, background_color.c_str());

        cairo_set_source_rgb(cr, color.red, color.green, color.blue);

        int x = element.x;
        int y = element.y - extents.y_bearing;

        cairo_move_to(cr, x, y);
        cairo_show_text(cr, element.text.c_str());
    } else {
        for(auto& child : element.childrens) {
            draw_element(widget, cr, child, theme);
        }
    }
}

static gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    current_cr = cr;

    auto theme_var = uvaapp->theme()->request("frame");

    if(!theme_var.is_a<var::dictionary>()) {
        return FALSE;
    }

    var::dictionary_type theme = theme_var.as<var::dictionary>();

    std::string background_color = theme["background"];

    guint width, height;
    GdkRGBA color;
    gdk_rgba_parse(&color, background_color.c_str());

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgb(cr, color.red, color.green, color.blue);

    cairo_rectangle(cr, 0, 0, width, height);

    cairo_fill(cr);

    uva::lang::ui::frame* frame = reinterpret_cast<uva::lang::ui::frame*>(data);

    auto view = frame->render();

    if(!view.is_a<var::dictionary>()) {
        return FALSE;
    }

    var::dictionary_type layout = view.as<var::dictionary>();

    view_element element(layout);
    element.width = width;
    element.height = height;
    
    element.calculate_layout();
    
    // Draw elements
    for(auto& child : element.childrens) {
        draw_element(widget, cr, child, theme);
    }
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

    g_signal_connect (drawing_area, "draw", G_CALLBACK (draw_callback), this);
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