#include <lang/lang.hpp>

#include <filesystem>

#include <uva/file.hpp>

#include <interpreter/interpreter.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::file_class::create(uva::lang::interpreter* interpreter)
{
    auto FileClass = std::make_shared<uva::lang::structure>("File");

    FileClass->methods = {
        { "read", uva::lang::method("read", method_storage_type::class_method, {"path"}, [interpreter](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string& input_path = params[0]->as<std::string>();
            std::filesystem::path path = std::filesystem::absolute(input_path);

            if(!std::filesystem::exists(path)) {
                throw std::runtime_error("file '" + path.string() + "' does not exist");
            }

            return uva::lang::object::instantiate(interpreter->StringClass, std::move(uva::file::read_all_text<char>(path)));
        })},
        { "read_all_lines", uva::lang::method("read_all_lines", method_storage_type::class_method, {"path"}, [interpreter](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            const std::string& input_path = params[0]->as<std::string>();
            std::filesystem::path path = std::filesystem::absolute(input_path);

            if(!std::filesystem::exists(path)) {
                throw std::runtime_error("file '" + path.string() + "' does not exist");
            }

            std::vector<std::string> file = uva::file::read_all_lines<char>(path);

            std::vector<std::shared_ptr<uva::lang::object>> lines;

            for(auto& line : file) {
                lines.push_back(uva::lang::object::instantiate(interpreter->StringClass, std::move(line)));
            }

            return uva::lang::object::instantiate(interpreter->ArrayClass, std::move(lines));
        })},
    };
    
    return FileClass;
}