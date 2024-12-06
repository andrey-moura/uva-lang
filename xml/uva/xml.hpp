#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <uva/color.hpp>

namespace uva
{
    class enumeration
    {
    public:
        enumeration(std::string_view name, std::vector<std::string_view> keys)
            : name(name), keys(std::move(keys))
        {
            for(size_t i = 0; i < this->keys.size(); i++) {
                values[this->keys[i]] = i;
            }
        }
    public:
        std::string_view name;
        std::vector<std::string_view> keys;
        std::map<std::string_view, size_t> values;
    };
    class xml
    {
    public:
        class type
        {
        public:
            std::string_view name;
            std::string_view pattern_restriction;
            std::vector<std::string_view> enum_restriction;
        };
        class element_attribute
        {
        public:
            std::string_view name;
            std::string_view type;
            std::string_view default_value;
            
            enum use
            {
                required,
                optional,
                prohibited,
            } use;
        };
        class element
        {
        public:
            std::vector<type> types;
            std::string_view name;
            std::vector<element_attribute> attributes;
        };
        class schema
        {
        public:
            schema(uva::xml xml);
        public:
            int integer_attribute(const uva::xml& xml, std::string_view a);
            uva::color color_attribute(const uva::xml& xml, std::string_view a);
        protected:
            std::shared_ptr<uva::xml> source;
            std::vector<type> types;
            std::vector<element> elements;
        };
    public:
        xml() = default;
    public:
        static xml decode(const char* xml_source);
        static xml decode(std::string_view xml_source);
        static xml decode(std::string xml_source);
    private:
        static bool is_comment(const char* it, const char* end);
        static void ignore_comment(const char*& it, const char* end);
    protected:
        // If you want to debug, uncomment the following line and comment the line below.
        //using string = std::string;
        using string = std::string_view;
    public:
        std::shared_ptr<xml> declaration;
        std::shared_ptr<xml> model;

        std::string_view tag;
        std::string source;
        std::string_view source_view;
        std::vector<xml> childrens;
        std::string_view content;
        std::map<string, string> attributes;
    public:
        /// @brief Try to get an attribute from the xml.
        /// @param key The key of the attribute.
        /// @return The attribute if it exists, otherwise an empty string. The existing attribute can be empty.
        string try_attribute(string key);

        /// @brief Get an attribute from the xml.
        /// @param key The key of the attribute.
        /// @param default The default value if the attribute does not exist.
        /// @return The attribute if it exists, otherwise the default value.
        string attribute(string key, string __default = "") const;

        /// @brief Get an attribute from the xml as an integer.
        /// @param key The key of the attribute.
        /// @param e   The enumeration to use.
        /// @param value The output value of the attribute if it is valid.
        /// @param actual The actual value of the attribute.
        /// @return True if the attribute exists and is valid, otherwise false.
        bool enumerate_attribute(string key, const enumeration& e, size_t& value, const std::string_view** actual = nullptr) const;
    };
};