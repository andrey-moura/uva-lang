#include <andy/lang/lang.hpp>

#include <filesystem>

#include <uva/file.hpp>

#include <andy/lang/interpreter.hpp>

std::shared_ptr<andy::lang::structure> andy::lang::file_class::create(andy::lang::interpreter* interpreter)
{
    auto FileClass = std::make_shared<andy::lang::structure>("File");

    FileClass->methods = {
        { "read", andy::lang::method("read", method_storage_type::class_method, {"path"}, [interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            const std::string& input_path = params[0]->as<std::string>();
            std::filesystem::path path = std::filesystem::absolute(input_path);

            if(!std::filesystem::exists(path)) {
                throw std::runtime_error("file '" + path.string() + "' does not exist");
            }

            return andy::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(uva::file::read_all_text<char>(path)));
        })},
        { "read_all_lines", andy::lang::method("read_all_lines", method_storage_type::class_method, {"path"}, [interpreter](std::shared_ptr<andy::lang::object> object, std::vector<std::shared_ptr<andy::lang::object>> params) {
            const std::string& input_path = params[0]->as<std::string>();
            std::filesystem::path path = std::filesystem::absolute(input_path);

            if(!std::filesystem::exists(path)) {
                throw std::runtime_error("file '" + path.string() + "' does not exist");
            }

            std::vector<std::string> file = uva::file::read_all_lines<char>(path);

            std::vector<std::shared_ptr<andy::lang::object>> lines;

            for(auto& line : file) {
                lines.push_back(andy::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(line)));
            }

            return andy::lang::object::instantiate(interpreter, interpreter->ArrayClass, std::move(lines));
        })},
    };
    
    return FileClass;
}