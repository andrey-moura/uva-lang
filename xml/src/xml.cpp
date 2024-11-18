#include <uva/xml.hpp>

uva::xml uva::xml::decode(const std::string& xml)
{
    std::string_view xml_view = xml;
    return uva::xml::decode(xml_view);
}

uva::xml uva::xml::decode(std::string_view& xml_view)
{
    uva::xml xml;

    while(xml_view.size()) {
        while(xml_view.size() && isspace(xml_view[0])) {
            xml_view.remove_prefix(1);
        }

        if(!xml_view.starts_with('<')) {
            throw std::runtime_error("expected '<'");
        }

        xml_view.remove_prefix(1);

        while(xml_view.size() && !isspace(xml_view[0]) && xml_view[0] != '>') {
            xml.tag.push_back(xml_view[0]);
            xml_view.remove_prefix(1);
        }

        while(xml_view.size() && isspace(xml_view[0])) {
            xml_view.remove_prefix(1);
        }

        if(!xml_view.starts_with('>')) {
            // attributes
            while(xml_view.size() && xml_view[0] != '>') {
                std::string key;
                std::string value;

                while(xml_view.size() && isspace(xml_view[0])) {
                    xml_view.remove_prefix(1);
                }

                while(xml_view.size() && !isspace(xml_view[0]) && xml_view[0] != '=') {
                    key.push_back(xml_view[0]);
                    xml_view.remove_prefix(1);
                }

                while(xml_view.size() && isspace(xml_view[0])) {
                    xml_view.remove_prefix(1);
                }

                if(!xml_view.starts_with('=')) {
                    throw std::runtime_error("expected '='");
                }

                xml_view.remove_prefix(1);

                while(xml_view.size() && isspace(xml_view[0])) {
                    xml_view.remove_prefix(1);
                }

                if(xml_view.front() == '"') {
                    xml_view.remove_prefix(1);

                    while(xml_view.size() && xml_view[0] != '"') {
                        value.push_back(xml_view[0]);
                        xml_view.remove_prefix(1);
                    }

                    if(!xml_view.size()) {
                        throw std::runtime_error("expected '\"'");
                    }

                    xml_view.remove_prefix(1);
                } else {
                    while(xml_view.size() && !isspace(xml_view[0]) && xml_view[0] != '>') {
                        value.push_back(xml_view[0]);
                        xml_view.remove_prefix(1);
                    }
                }

                xml.attributes[key] = value;
            }
        }

        if(!xml_view.starts_with('>')) {
            throw std::runtime_error("expected '>'");
        }

        xml_view.remove_prefix(1);

        while(xml_view.size() && isspace(xml_view[0])) {
            xml_view.remove_prefix(1);
        }

        if(!xml_view.starts_with("</")) {
            while(xml_view.size() && !xml_view.starts_with("</")) {
                uva::xml child = uva::xml::decode(xml_view);
                xml.childrens.push_back(child);

                while(xml_view.size() && isspace(xml_view[0])) {
                    xml_view.remove_prefix(1);
                }
            }
        }

        while(xml_view.size() && isspace(xml_view[0])) {
            xml_view.remove_prefix(1);
        }

        if(!xml_view.starts_with("</")) {
            throw std::runtime_error("expected '</'");
        }

        xml_view.remove_prefix(2);

        if(!xml_view.starts_with(xml.tag)) {
            throw std::runtime_error("expected closing tag to match opening tag");
        }

        xml_view.remove_prefix(xml.tag.size());

        if(!xml_view.starts_with('>')) {
            throw std::runtime_error("expected '>'");
        }

        xml_view.remove_prefix(1);

        if(!xml_view.starts_with('<') || xml_view.starts_with("</")) {
            // no more siblings
            break;
        }
    }
    
    return xml;
}