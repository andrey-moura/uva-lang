#include <lang/lang.hpp>

#include <filesystem>

#include <uva/file.hpp>

#include <interpreter/interpreter.hpp>

std::shared_ptr<uva::lang::structure> uva::lang::file_class::create(uva::lang::interpreter* interpreter)
{
    auto FileClass = std::make_shared<uva::lang::structure>("File");

    FileClass->methods = {
        { "read", uva::lang::method("read", method_storage_type::class_method, {"path"}, [interpreter](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::shared_ptr<uva::lang::object> obj = std::make_shared<uva::lang::object>(interpreter->StringClass);

            const std::string& input_path = *(params[0]->as<std::string>());
            std::filesystem::path path = std::filesystem::absolute(input_path);

            if(!std::filesystem::exists(path)) {
                throw std::runtime_error("file '" + path.string() + "' does not exist");
            }

            std::string file = uva::file::read_all_text<char>(path);

            obj->native = new std::string(file);

            return obj;
        })},
        { "read_all_lines", uva::lang::method("read_all_lines", method_storage_type::class_method, {"path"}, [interpreter](uva::lang::object* object, std::vector<std::shared_ptr<uva::lang::object>> params) {
            std::shared_ptr<uva::lang::object> obj = std::make_shared<uva::lang::object>(interpreter->ArrayClass);

            const std::string& input_path = *(params[0]->as<std::string>());
            std::filesystem::path path = std::filesystem::absolute(input_path);

            if(!std::filesystem::exists(path)) {
                throw std::runtime_error("file '" + path.string() + "' does not exist");
            }

            std::vector<std::string> file = uva::file::read_all_lines<char>(path);

            std::vector<std::shared_ptr<uva::lang::object>>* lines = new std::vector<std::shared_ptr<uva::lang::object>>();

            for(auto& line : file) {
                std::shared_ptr<uva::lang::object> line_obj = std::make_shared<uva::lang::object>(interpreter->StringClass);
                line_obj->native = new std::string(std::move(line));

                lines->push_back(line_obj);
            }

            obj->native = (void*)lines;

            return obj;
        })},
    };
    
    return FileClass;
}