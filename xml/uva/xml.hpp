#pragma once

#include <string>
#include <vector>
#include <map>

namespace uva
{
    class xml
    {
    public:
        xml() = default;
        ~xml() = default;
    public:
        static xml decode(const std::string& xml);
        static xml decode(std::string_view& xml);
    public:
        std::string tag;
        std::string content;
        std::vector<xml> childrens;
        std::map<std::string, std::string> attributes;
        std::map<std::string, std::string> styles;
    };
};