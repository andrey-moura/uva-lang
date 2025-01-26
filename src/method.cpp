#include <uva/lang/method.hpp>
#include <uva/lang/object.hpp>
#include <uva/lang/class.hpp>

std::shared_ptr<uva::lang::object> uva::lang::method::call(std::shared_ptr<uva::lang::object> o)
{
    return function(o, std::vector<std::shared_ptr<uva::lang::object>>(), std::map<std::string, std::shared_ptr<uva::lang::object>>());
}

void uva::lang::method::init_params(std::vector<std::string> __params)
{
    for(auto& param : __params) {
        fn_parameter fn_param(std::move(param));
        if(fn_param.named) {
            named_params.push_back(std::move(fn_param));
        } else {
            positional_params.push_back(std::move(fn_param));
        }
    }
}
