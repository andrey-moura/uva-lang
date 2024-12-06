#pragma once

#include <string>
#include <string_view>
#include <filesystem>

#include <uva/size.hpp>
#include <uva/color.hpp>
#include <uva/rect.hpp>

#include "os_specific_data_member.hpp"

namespace uva
{
    //temp
    namespace drawing
    {
        class window_surface;
        class window : private os_specific_data_member<8>
        {
        public:
            /// @brief Create a new window with the specified title and the OS's default size and position.
            /// @param __title The title of the window.
            window(std::string_view __title);
        public:
            /// @brief Show the window.
            /// @param maximized Whether the window should be maximized.
            void show(bool maximized = false);
            /// @brief Hide the window.
            void hide();
        public:
            window_surface create_surface();
        };
        class surface : public os_specific_data_member<8>
        {
        public:
            surface(const uva::size& s)
                : m_size(s)
            {
            }
            surface(size_t __width, size_t __height)
                : m_size(__width, __height)
            {
            }
        // Protected members
        protected:
            uva::size m_size;
        // Acessors
        public:
            /// @brief Get the size of the surface.
            const uva::size& size() const { return m_size; }
            /// @brief Get the width of the surface.
            size_t width() const { return m_size.w; }
            /// @brief Get the height of the surface.
            size_t height() const { return m_size.h; }
        public:
            /// @brief Write the surface to a file. The format is os specific.
            /// @param path The path to write the file to. The path must be a directory. It will be updated with the file name and extension.
            /// @param stem The stem of the file.
            virtual bool write_to_file(std::filesystem::path& path, std::string_view stem) { return false; }
        };
        class window_surface : public surface
        {
        };
        class memory_surface : public surface
        {
        public:
            memory_surface(size_t __width, size_t __height);
            ~memory_surface();
        public:
            /// @brief Write the surface to a file. The format is os specific.
            /// @param path The path to write the file to. The path must be a directory. It will be updated with the file name and extension.
            /// @param stem The stem of the file.
            virtual bool write_to_file(std::filesystem::path& path, std::string_view stem) override;
        };
        class texture_surface : public surface
        {
        public:
            texture_surface(const uva::size& s);
            ~texture_surface();
        };
        class basic_renderer : protected os_specific_data_member<8>
        {
        public:
            basic_renderer(surface& __surface)
                : m_surface(__surface)
            {
            }
            ~basic_renderer() = default;
        public:
            virtual void fill_rect(const uva::rect& __rect, const uva::color& __color) { }
            virtual void clear(const uva::color& __color) { }
        private:
            surface& m_surface;
        };
        class software_renderer : public basic_renderer
        {
        public:
            software_renderer(surface& __surface);
            ~software_renderer();
        public:
            void fill_rect(const uva::rect& __rect, const uva::color& __color) override;
            void clear(const uva::color& __color) override;
        };
        class hardware_renderer : public basic_renderer
        {
        public:
            hardware_renderer(surface& __surface);
            ~hardware_renderer();
        public:
            void fill_rect(const uva::rect& __rect, const uva::color& __color) override;
            void clear(const uva::color& __color) override;
        };
    };
};