#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <uva/file.hpp>
#include <console.hpp>

#include <extension/extension.hpp>
#include <interpreter/interpreter.hpp>
#include <lang/lang.hpp>

#include <SDL2/SDL.h>

std::vector<uva::lang::extension*> extensions;

#ifndef _NDEBUG
    #define try if(true)
    #define catch(e) if(false)

    std::exception e;
#endif

std::map<void*, void*> s_frames;

namespace uva
{
    namespace lang
    {
        namespace ui
        {
            class window
            {
            private:
                SDL_Window* m_frame = nullptr;
                
                SDL_Surface* m_surface = nullptr;
                SDL_Renderer* m_renderer = nullptr;
            public:
                window(const std::string& title)
                {
                    m_frame = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);

                    if(!m_frame) {
                        throw std::runtime_error("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
                    }

                    m_renderer = SDL_CreateRenderer(m_frame, -1, SDL_RENDERER_SOFTWARE);

                    s_frames[m_frame] = this;
                }

                ~window()
                {
                    SDL_DestroyWindow(m_frame);
                }

                void show(bool maximized = false)
                {
                    if(maximized) {
                        SDL_MaximizeWindow(m_frame);
                    }

                    SDL_ShowWindow(m_frame);
                }

                void hide()
                {
                    SDL_HideWindow(m_frame);
                }

                void draw()
                {
                    SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255);
                    SDL_RenderClear(m_renderer);

                    SDL_RenderPresent(m_renderer);
                }

                void event(SDL_WindowEvent& event)
                {
                    switch (event.event)
                    {
                    case SDL_WINDOWEVENT_EXPOSED:
                        draw();
                        break;
                    default:
                        break;
                    }
                }
            };
        };
    };
};

int main(int argc, char** argv) {
    int result = SDL_Init(SDL_INIT_VIDEO);

    if(result != 0) {
        uva::console::log_error("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    try {
        uva::lang::interpreter interpreter;

        std::shared_ptr<uva::lang::structure> ui_application_class = std::make_shared<uva::lang::structure>("UI.Application");
        std::shared_ptr<uva::lang::structure> ui_window_class = std::make_shared<uva::lang::structure>("UI.Frame");
        ui_window_class->methods = {
            { "new", uva::lang::method("new", uva::lang::method_storage_type::instance_method, {"title"}, [&interpreter, ui_window_class](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params){
                std::string title = params[0]->as<std::string>();

                uva::lang::ui::window* window = new uva::lang::ui::window(title);

                object->set_native_ptr(window);

                return nullptr;
            })},
            { "show", uva::lang::method("show", uva::lang::method_storage_type::instance_method, {"maximized"}, [&interpreter](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params){
                uva::lang::object* maximized = params[0].get();
                uva::lang::ui::window& window = object->as<uva::lang::ui::window>();
                window.show(maximized && maximized->is_present());

                return nullptr;
            })},
            { "hide", uva::lang::method("hide", uva::lang::method_storage_type::instance_method, {}, [&interpreter](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params){
                uva::lang::ui::window& window = object->as<uva::lang::ui::window>();
                window.hide();

                return nullptr;
            })},
        };

        interpreter.load(ui_application_class);
        interpreter.load(ui_window_class);

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

        interpreter.execute_all(root_node);

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

        auto run_it = application_class->methods.find("run");

        if(run_it == application_class->methods.end()) {
            throw std::runtime_error("run method not defined in class Application. Define it so uva know where to start the application");
        }

        interpreter.call(nullptr, nullptr, run_it->second, {});

        SDL_Event event;
        bool running = true;

        while(running) {
            while(SDL_PollEvent(&event)) {
                switch (event.type)
                {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_WINDOWEVENT: {
                    SDL_Window* window = SDL_GetWindowFromID(event.window.windowID);

                    if(window) {
                        uva::lang::ui::window* frame = (uva::lang::ui::window*)s_frames[window];
                        frame->event(event.window);
                    }
                    break;
                }
                default:
                    break;
                }
            }
        }

        return 0;
    } catch(const std::exception& e) {
        uva::console::log_error(e.what());
    }

    return 0;
}