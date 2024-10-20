#include "parser.hpp"

#include <uva/file.hpp>

#include <exception>
#include <iostream>

using namespace uva;
using namespace lang;

std::shared_ptr<uva::lang::structure> parser::parse(const std::filesystem::path &path)
{
    std::shared_ptr<uva::lang::structure> c;

    std::string class_content = uva::file::read_all_text<char>(path);
    std::string_view content_view = class_content;

    uva::lang::lexer::cursor cursor(content_view);

    while(!cursor.eof()) {
        uva::console::log_debug("read cursor of type {} at {} with content '{}'", (int)cursor.type(), cursor.human_start_position(), cursor.content());

        if(cursor.type() == uva::lang::lexer::cursor_type::cursor_class) {
            c = std::make_shared<uva::lang::structure>(std::string(cursor.decname()));
            c->source_content = std::move(class_content);

            for(auto& child : cursor.block()->children()) {
                if(child.type() == uva::lang::lexer::cursor_type::cursor_function) {
                    Method m;
                    m.name = std::string(child.decname());
                    m.block = std::string(child.content());
                    m.block_cursor = *child.block();

                    c->methods[m.name] = m;
                }
            }
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