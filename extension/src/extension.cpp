#include "extension/extension.hpp"

#include <iostream>

extern std::vector<uva::lang::extension*> extensions;

uva::lang::extension::extension(const std::string &name)
    : m_name(name)
{
    extensions.push_back(this);
}

void uva::lang::extension::init_all()
{
    for(auto& extension : extensions) {
        extension->init();
    }
}