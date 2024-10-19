#include <string>
#include <vector>
#include <map>
#include <memory>

#include <uva/var.hpp>

namespace uva {
    namespace lang {
        class Object;
        class Method;

        struct structure
        {
            //for user code, use create
            structure(const std::string& __name, std::vector<Method> __methods = {});
            ~structure();
        public:
            std::string name;
            std::map<std::string, Method> methods;
            std::shared_ptr<structure> base;
            std::map<std::string, std::shared_ptr<Object>> variables;
            std::string source_content;

            std::shared_ptr<Object> call(const Method& method, const var& params= null);
            std::shared_ptr<Object> call(const std::string& method, const var& params = null)
            {
                return call(methods[method], params);
            }
        };
    };
};