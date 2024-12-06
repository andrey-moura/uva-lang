#include <uva/xml.hpp>

#include <uva/binary.hpp>

uva::xml::schema::schema(uva::xml __xml)
    : source(std::make_shared<uva::xml>(std::move(__xml)))
{
    auto parse_type = [](const uva::xml& xml) {
        type t;
        t.name = xml.attribute("name");

        for(auto& possible_restriction : xml.childrens) {
            if(possible_restriction.tag == "xs:restriction") {
                for(auto& restriction_child : possible_restriction.childrens) {
                    if(restriction_child.tag == "xs:pattern") {
                        t.pattern_restriction = restriction_child.attribute("value");
                    } else if(restriction_child.tag == "xs:enumeration") {
                        t.enum_restriction.push_back(restriction_child.attribute("value"));
                    }
                }
            }
        }

        return t;
    };

    for(auto& child : source->childrens) {
        if(child.tag == "xs:simpleType") {
            type t = parse_type(child);
            types.push_back(t);
        } else if(child.tag == "xs:element") {
            element e;
            e.name = child.attribute("name");

            for(auto& element_child : child.childrens) {
                if(element_child.tag == "xs:complexType") {
                    for(auto& complex_child : element_child.childrens) {
                        if(complex_child.tag == "xs:attribute") {
                            element_attribute a;
                            a.name = complex_child.attribute("name");
                            a.type = complex_child.attribute("type");
                            a.default_value = complex_child.attribute("default");

                            std::string_view use = complex_child.attribute("use");

                            if(use == "required") {
                                a.use = element_attribute::required;
                            } else if(use == "optional") {
                                a.use = element_attribute::optional;
                            } else if(use == "prohibited") {
                                a.use = element_attribute::prohibited;
                            }

                            if(complex_child.childrens.size()) {
                                for(auto& attribute_child : complex_child.childrens) {
                                    if(attribute_child.tag == "xs:simpleType") {
                                        type t = parse_type(attribute_child);
                                        t.name = complex_child.attribute("name");

                                        e.types.push_back(t);
                                    }
                                }
                            }

                            e.attributes.push_back(a);
                        }
                    }
                }
            }

            elements.push_back(e);
        }
    }
}

int uva::xml::schema::integer_attribute(const uva::xml& xml, std::string_view a)
{
    std::string_view value = xml.attribute(a);

    for(const auto& element : elements) {
        if(element.name == xml.tag) {
            for(const auto& attribute : element.attributes) {
                if(attribute.name == a) {
                    for(const auto& type : element.types) {
                        if(type.name == attribute.name) {
                            if(value.empty()) {
                                value = attribute.default_value;
                            }

                            if(type.enum_restriction.size()) {
                                if(auto enum_it = std::find(type.enum_restriction.begin(), type.enum_restriction.end(), value); enum_it != type.enum_restriction.end()) {
                                    return std::distance(type.enum_restriction.begin(), enum_it);
                                }
                            }
                        }
                    }

                    for(const auto& type : types) {
                        if(type.name == attribute.type) {
                            if(value.empty()) {
                                value = attribute.default_value;
                            }

                            if(type.enum_restriction.size()) {
                                if(auto enum_it = std::find(type.enum_restriction.begin(), type.enum_restriction.end(), value); enum_it != type.enum_restriction.end()) {
                                    return std::distance(type.enum_restriction.begin(), enum_it);
                                }
                            }
                        }
                    }
                    
                    break;
                }

            }
        }
    }

    throw std::runtime_error("attribute not found");
}

uva::color uva::xml::schema::color_attribute(const uva::xml& xml, std::string_view a)
{
    std::string_view value = xml.attribute(a);

    for(const auto& element : elements) {
        if(element.name == xml.tag) {
            for(const auto& attribute : element.attributes) {
                if(attribute.name == a) {
                    if(value.empty()) {
                        value = attribute.default_value;
                    }

                    if(value.size()) {
                        if(value.starts_with("#")) {
                            if(value.size() != 7) {
                                throw std::runtime_error("color must be a hex color");
                            }

                            uva::color c;

                            c.r = uva::binary::byte_from_hex_string(value.data() + 1);
                            c.g = uva::binary::byte_from_hex_string(value.data() + 3);
                            c.b = uva::binary::byte_from_hex_string(value.data() + 5);
                            c.a = 255;

                            return c;
                        }
                    }
                }
            }
        }
    }

    throw std::runtime_error("attribute not found");
}