#include "parser.hpp"

#include <uva/file.hpp>

#include <exception>
#include <iostream>

using namespace uva;
using namespace lang;

std::map<std::string, void(*)(uva::parser* parser, std::shared_ptr<uva::lang::vm> vm, var params)> uva::parser::parser_funtions = {

    {"require_directory", [](uva::parser* parser, std::shared_ptr<uva::lang::vm> vm, var params) {
        size_t params_size = params.size();

        if(params.size() != 1) {
            throw std::runtime_error("require_directory: wrong number of arguments. Expected 1, got " + std::to_string(params_size));
        }

        std::filesystem::path directory = parser->absolute(params[0]);

        if(!std::filesystem::exists(directory)) {
            throw std::runtime_error("require_directory: directory does not exist");
        }

        for(auto& entry : std::filesystem::directory_iterator(directory)) {
            if(entry.is_regular_file()) {
                std::string extension = entry.path().extension().string();

                if(extension == ".uva") {
                    uva::parser new_parser;
                    new_parser.parse(entry.path(), vm);
                }
            }
        }
    }}
};

std::shared_ptr<uva::lang::structure> parser::parse(const std::filesystem::path &path, std::shared_ptr<uva::lang::vm> vm_instance)
{
    current_path = path;
    current_path.remove_filename();

    std::shared_ptr<uva::lang::structure> c;

    std::string class_content = uva::file::read_all_text<char>(path);
    std::string_view content_view = class_content;

    uva::lang::lexer::cursor cursor(content_view);

    while(!cursor.eof()) {
        uva::console::log_debug("read cursor of type {} at {} with content '{}'", (int)cursor.type(), cursor.human_start_position(), cursor.is_undefined() ? "undefined" : cursor.content());

        switch(cursor.type()) {
            case uva::lang::lexer::cursor_type::cursor_fncall: {
                // functions calls outside of classes are allowed only if it is a parser function.
                auto it = parser_funtions.find(std::string(cursor.decname()));

                if(it == parser_funtions.end()) {
                    cursor.throw_error_at_current_position("parser error: function call is not allowed outside of class");
                }

                var params = var::array();

                const uva::lang::lexer::cursor* params_cursor = cursor.fncallparams();

                if(params_cursor) {
                    for(auto& param : params_cursor->children()) {
                        std::string_view content = param.content();
                        if(content.starts_with("\"") && content.ends_with("\"")) {
                            content.remove_prefix(1);
                            content.remove_suffix(1);
                            
                            params.push_back(std::string(content));
                        }
                    }
                }

                it->second(this, vm_instance, params);
            }
            break;
            case uva::lang::lexer::cursor_type::cursor_class: {
                c = std::make_shared<uva::lang::structure>(std::string(cursor.decname()));
                vm_instance->load(c);
                c->source_content = std::move(class_content);

                for(auto& child : cursor.block()->children()) {
                    switch(child.type()) {
                        case uva::lang::lexer::cursor_type::cursor_function: {
                            Method m;
                            m.name = std::string(child.decname());
                            m.block = std::string(child.content());
                            m.block_cursor = *child.block();

                            c->methods[m.name] = m;
                        }
                        break;
                    }
                }
            }
            break;
        }

        cursor = cursor.parse_next();
    }

    uva::console::log_debug("read cursor of type eof at {}", cursor.human_start_position());

    return c;
}

uva::lang::object::~object()
{
    uva::console::log_debug("{}#{} destroyed", cls->name, (void*)this);
}

uva::lang::structure::~structure()
{
    uva::console::log_debug("{}#Class destroyed", name);
}


uva::lang::structure::structure(const std::string& __name, std::vector<Method> __methods)
    : name(__name)
{
    uva::console::log_debug("{}#Class created", name);
}