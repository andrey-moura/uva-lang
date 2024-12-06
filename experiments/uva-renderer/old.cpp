    
    #include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("uva", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_ShowWindow(window);

    SDL_Renderer* renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Event event;
    bool running = true;

    size_t total_frames = 0;
    size_t frames_last_second = 0;
    auto very_start = std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds total_duration = std::chrono::milliseconds(0);
    size_t next_duration = 1000;
    const float target_fps = 60.0;
    const float target_frame_time_ms = 1000.0 / target_fps;
    const std::chrono::nanoseconds target_frame_nanoseconds = std::chrono::nanoseconds((int)(1000000000 / target_fps));
    float total_free_time_ms = 0.0;

    while(running) {
        auto frame_start = std::chrono::high_resolution_clock::now();

        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    running = false;
                break;
                //window
                case SDL_WINDOWEVENT: {
                    //std::cout << "drawing" << std::endl;

                    auto draw_start = std::chrono::high_resolution_clock::now();

                    std::string content = uva::file::read_all_text<char>(std::filesystem::absolute("test.xml"));

                    // std::cout << content << std::endl;

                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                    SDL_RenderClear(renderer);

                    if(content.size()) {
                        int width, height;
                        SDL_GetWindowSize(window, &width, &height);

                        //std::cout << "file read" << std::endl;

                        uva::xml xml = uva::xml::decode(content);

                        //std::cout << "xml decoded" << std::endl;

                        //std::cout << "found " << xml.childrens.size() << " childreans" << std::endl;

                        auto parse_element = [](view_element* ve, uva::xml& xml) {
                            auto background_color = xml.try_attribute("background-color");

                            if(background_color.size()) {
                                if(background_color.starts_with("#")) {
                                    if(background_color.size() != 9) {
                                        throw std::runtime_error("background-color must be a hex color");
                                    }

                                    ve->style.background_color.r = uva::binary::byte_from_hex_string(background_color.data() + 1);
                                    ve->style.background_color.g = uva::binary::byte_from_hex_string(background_color.data() + 3);
                                    ve->style.background_color.b = uva::binary::byte_from_hex_string(background_color.data() + 5);
                                }
                            }

                            ve->style.flex = atoi(xml.attribute("flex", "0").data());
                        };

                        auto draw_element = [](view_element* ve, SDL_Renderer* renderer) {
                            SDL_SetRenderDrawColor(renderer, ve->style.background_color.r, ve->style.background_color.g, ve->style.background_color.b, ve->style.background_color.a);
                            SDL_Rect rect = { ve->x, ve->y, ve->w, ve->h };
                            SDL_RenderFillRect(renderer, &rect);

                            // check if it is a text element

                        };

                        if(xml.tag == "layout") {
                            layout l;
                            parse_element(&l, xml);
                            std::string_view type = xml.try_attribute("type");

                            if(type.empty()) {
                                throw std::runtime_error("layout must have a type");
                            }

                            auto parse_from_xml = [](const uva::xml& xml, const uva::enumeration& enumeration) {
                                size_t output;
                                if(!xml.enumerate_attribute(enumeration.name, enumeration, output)) {
                                    throw std::runtime_error("invalid enumeration");
                                }
                                return (size_t)output;
                            };

                            l.style.type      = (layout_style::layout_type)parse_from_xml(xml, layout_style::layout_type_enumeration);
                            l.style.direction = (layout_style::layout_flex_direction)parse_from_xml(xml, layout_style::layout_flex_direction_enumeration);

                            for(auto& child : xml.childrens) {
                                if(child.tag == "text") {
                                    text_element te;
                                    te.content = child.content;
                                    parse_element(&te, child);
                                    l.childreans.push_back(te);

                                    // Get text size
                                    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/abyssinica/AbyssinicaSIL-Regular.ttf", 24);
                                    if(!font) {
                                        throw std::runtime_error(SDL_GetError());
                                    }
                                    SDL_Color color = { 255, 255, 255, 255 };
                                    TTF_SizeText(font, te.content.c_str(), &te.w, &te.h);
                                    TTF_CloseFont(font);
                                }
                            }

                            l.calculate_layout(0, 0, width, height);

                            draw_element(&l, renderer);

                            for(auto& child : l.childreans) {
                                draw_element(&child, renderer);          
                            }
                        }
                    }

                    SDL_RenderPresent(renderer);
                }
                break;
            }
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