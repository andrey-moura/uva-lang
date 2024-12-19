#include <memory>

namespace uva
{
    namespace lang
    {
        class interpreter;
        class structure;

        namespace app_class
        {
            std::shared_ptr<uva::lang::structure> create(uva::lang::interpreter* interpreter);
        };
    };
};