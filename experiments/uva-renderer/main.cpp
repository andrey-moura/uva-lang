#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>

#include <uva/file.hpp>
#include <uva/xml.hpp>
#include <uva/binary.hpp>

#include "layout.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>

SDL_Renderer* renderer;

void parse_element(uva::xml::schema& schema, view_element* ve, uva::xml& xml) {
    // For text element
    text_element* te = dynamic_cast<text_element*>(ve);
    layout* le = dynamic_cast<layout*>(ve);
    
    if(te) {
        te->style.color = schema.color_attribute(xml, "color");
        te->style.cursor = (view_element_cursor)schema.integer_attribute(xml, "cursor");

        te->text_style.vertical_alignment   = (text_vertical_alignment)schema.integer_attribute(xml, "vertical-align");
        te->text_style.horizontal_alignment = (text_horizontal_alignment)schema.integer_attribute(xml, "horizontal-align");
    } else if(le) {
        le->layout_style.type = (layout_element_style::layout_type)schema.integer_attribute(xml, "type");
        le->layout_style.direction = (layout_element_style::layout_flex_direction)schema.integer_attribute(xml, "direction");

        for(auto& child : xml.childrens) {
            if(child.tag == "text") {
                std::shared_ptr<text_element> te = std::make_shared<text_element>();
                te->content = child.content;
                
                parse_element(schema, te.get(), child);

                // Get text size
                TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/abyssinica/AbyssinicaSIL-Regular.ttf", 24);

                if(!font) {
                    throw std::runtime_error(SDL_GetError());
                }

                TTF_SizeText(font, te->content.c_str(), &te->w, &te->h);
                TTF_CloseFont(font);

                le->childreans.push_back(te);
            } else if(child.tag == "layout") {
                std::shared_ptr<layout> le2 = std::make_shared<layout>();
                parse_element(schema, le2.get(), child);

                le->childreans.push_back(le2);
            }
        }
    }


    ve->style.background_color = schema.color_attribute(xml, "background-color");

    ve->style.flex = atoi(xml.attribute("flex", "0").data());
    ve->style.padding = atoi(xml.attribute("padding", "0").data());
    ve->style.gap = atoi(xml.attribute("gap", "0").data());
    ve->style.border_radius = atoi(xml.attribute("border-radius", "0").data());
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("uva", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_ShowWindow(window);

    renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Event event;
    bool running = true;

    size_t total_frames = 0;
    size_t frames_last_second = 0;
    size_t total_draws = 0;
    auto very_start = std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds total_duration = std::chrono::milliseconds(0);
    size_t next_duration = 1000;
    const float target_fps = 60.0;
    const float target_frame_time_ms = 1000.0 / target_fps;
    const std::chrono::nanoseconds target_frame_nanoseconds = std::chrono::nanoseconds((int)(1000000000 / target_fps));
    float total_free_time_ms = 0.0;
    std::chrono::milliseconds total_draw_duration = std::chrono::milliseconds(0);

    layout l;

    while(running) {
        auto frame_start = std::chrono::high_resolution_clock::now();

        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    running = false;
                break;
                //window
                case SDL_WINDOWEVENT: {
                    auto start = std::chrono::high_resolution_clock::now();
                    //std::cout << "drawing" << std::endl;

                    auto draw_start = std::chrono::high_resolution_clock::now();

                    std::string content = uva::file::read_all_text<char>(std::filesystem::absolute("test.xml"));
                    std::string schema_content = uva::file::read_all_text<char>(std::filesystem::absolute("file.xsd"));

                    // std::cout << content << std::endl;

                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                    SDL_RenderClear(renderer);

                    if(content.size()) {
                        int width, height;
                        SDL_GetWindowSize(window, &width, &height);

                        //std::cout << "file read" << std::endl;

                        uva::xml xml = uva::xml::decode(std::move(content));
                        uva::xml::schema schema(std::move(uva::xml::decode(std::move(schema_content))));

                        //std::cout << "xml decoded" << std::endl;

                        //std::cout << "found " << xml.childrens.size() << " childreans" << std::endl;

                        if(xml.tag == "layout") {
                            l.childreans.clear();

                            parse_element(schema, &l, xml);

                            l.calculate_layout(0, 0, width, height);

                            l.draw();
                        }
                    }

                    SDL_RenderPresent(renderer);

                    auto draw_end = std::chrono::high_resolution_clock::now();
                    ++total_draws;

                    total_draw_duration += std::chrono::duration_cast<std::chrono::milliseconds>(draw_end - draw_start);
                }
                break;
            }
        }

        // Check if the window is in focus
        if(SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS) {
            //std::cout << "window in focus" << std::endl;

            int x, y;
            SDL_GetMouseState(&x, &y);

            // Try to find the element under the mouse

            view_element* element_at_mouse = nullptr;

            SDL_SystemCursor cursor = SDL_SYSTEM_CURSOR_ARROW;

            for(auto& child : l.childreans) {
                if(x >= child->x && x <= child->x + child->w && y >= child->y && y <= child->y + child->h) {
                    element_at_mouse = child.get();
                    
                    auto le = dynamic_cast<layout*>(child.get());

                    if(le) {
                        for(auto& child2 : le->childreans) {
                            if(x >= child2->x && x <= child2->x + child2->w && y >= child2->y && y <= child2->y + child2->h) {
                                element_at_mouse = child2.get();
                                break;
                            }
                        }
                    }
                }
            }

            if(element_at_mouse) {
                switch(element_at_mouse->style.cursor) {
                    case view_element_cursor::view_element_cursor_pointer:
                        cursor = SDL_SYSTEM_CURSOR_HAND;
                        break;
                    default:
                        cursor = SDL_SYSTEM_CURSOR_ARROW;
                        break;
                }
            }

            SDL_SetCursor(SDL_CreateSystemCursor(cursor));
        }

        auto now = std::chrono::high_resolution_clock::now();

        total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - very_start);

        ++total_frames;
        ++frames_last_second;

        if(total_duration.count() >= next_duration) {
            next_duration += 1000;

            float fps = total_frames / (total_duration.count() / 1000.0);

            std::cout << "total duration (ms): " << total_duration.count() << std::endl;
            std::cout << "total frames: " << total_frames << std::endl;
            std::cout << "target frame time (ms): " << target_frame_time_ms << std::endl;
            float avg_frame_time = total_duration.count() / (float)total_frames;
            float percent_frame_time = (avg_frame_time  / target_frame_time_ms) * 100.0;
            std::cout << "AVG frame time (ms): " << total_duration.count() / (float)total_frames << " (" << percent_frame_time << "%)" << std::endl;
            std::cout << "fps: " << fps << std::endl;
            std::cout << "fps last second: " << frames_last_second << std::endl;
            std::cout << "AVG free time (ms): " << total_free_time_ms / total_frames << std::endl;
            std::cout << "total draws: " << total_draws << std::endl;
            std::cout << "AVG draw time (ms): " << total_draw_duration.count() / (float)total_draws << std::endl;
            std::cout << std::endl;

            frames_last_second = 0;
        }

        auto frame_end = std::chrono::high_resolution_clock::now();

        auto frame_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(frame_end - frame_start);
        
        if(frame_duration < target_frame_nanoseconds) {
            std::chrono::nanoseconds free_time = target_frame_nanoseconds - frame_duration;
            int free_time_nano = free_time.count();
            total_free_time_ms += free_time_nano / 1000000.0;

            std::this_thread::sleep_for(free_time);
        }
    }

    return 0;
}

void view_element::draw()
{
    if(style.background_color.a == 0) {
        return;
    }

    SDL_SetRenderDrawColor(renderer, style.background_color.r, style.background_color.g, style.background_color.b, style.background_color.a);

    SDL_Rect rect = { x, y, w, h };

    if(style.border_radius) {
        rect.x += style.border_radius;
        rect.w -= 2 * style.border_radius;
    }

    SDL_RenderFillRect(renderer, &rect);

    if(style.border_radius) {
        rect.x = x;
        rect.y = style.border_radius + y;
        rect.h = h - 2 * style.border_radius;
        rect.w = style.border_radius;

        SDL_RenderFillRect(renderer, &rect);

        rect.x = x + w - style.border_radius;
        rect.y = style.border_radius + y;
        rect.h = h - 2 * style.border_radius;
        rect.w = style.border_radius;

        SDL_RenderFillRect(renderer, &rect);

        filledCircleRGBA(renderer, x + style.border_radius, y + style.border_radius, style.border_radius, style.background_color.r, style.background_color.g, style.background_color.b, style.background_color.a);
        filledCircleRGBA(renderer, (x - 1) + w - style.border_radius, y + style.border_radius, style.border_radius, style.background_color.r, style.background_color.g, style.background_color.b, style.background_color.a);
        // Bottom left
        filledCircleRGBA(renderer, x + style.border_radius, y + h - style.border_radius, style.border_radius, style.background_color.r, style.background_color.g, style.background_color.b, style.background_color.a);
        filledCircleRGBA(renderer, (x - 1) + w - style.border_radius, y + h - style.border_radius, style.border_radius, style.background_color.r, style.background_color.g, style.background_color.b, style.background_color.a);
    }

}

void layout::draw()
{
    view_element::draw();

    for(auto& child : childreans) {
        child->draw();
    }
}

void text_element::draw()
{
    view_element::draw();

    if(content.size()) {
        TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/abyssinica/AbyssinicaSIL-Regular.ttf", 24);

        if(!font) {
            throw std::runtime_error(SDL_GetError());
        }

        SDL_Color color = { style.color.r, style.color.g, style.color.b, style.color.a };
        SDL_Surface* surface = TTF_RenderText_Solid(font, content.c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        // Get size of the texture
        int texture_w, texture_h;
        SDL_QueryTexture(texture, NULL, NULL, &texture_w, &texture_h);
        SDL_FreeSurface(surface);
        SDL_Rect rect = { x, y, texture_w, texture_h };
        switch(text_style.vertical_alignment) {
            case text_vertical_alignment::text_vertical_alignment_top:
                break;
            case text_vertical_alignment::text_vertical_alignment_center:
                rect.y += (h - texture_h) / 2;
                break;
            case text_vertical_alignment::text_vertical_alignment_bottom:
                rect.y += h - texture_h;
                break;
        }
        switch(text_style.horizontal_alignment) {
            case text_horizontal_alignment::text_horizontal_alignment_left:
                break;
            case text_horizontal_alignment::text_horizontal_alignment_center:
                rect.x += (w - texture_w) / 2;
                break;
            case text_horizontal_alignment::text_horizontal_alignment_right:
                rect.x += w - texture_w;
                break;
        }
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_DestroyTexture(texture);
        TTF_CloseFont(font);
    }
}