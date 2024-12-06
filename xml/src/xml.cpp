#include <uva/xml.hpp>
#include <string.h>

uva::xml uva::xml::decode(std::string xml_source)
{
    uva::xml xml = uva::xml::decode(std::string_view(xml_source));
    xml.source = std::move(xml_source);
    return xml;
}

uva::xml uva::xml::decode(const char *xml_source)
{
    return uva::xml::decode(std::string_view(xml_source));
}

bool uva::xml::is_comment(const char* it, const char* end)
{
    return (end - it) >= 3 && it[0] == '<' && it[1] == '!' && it[2] == '-' && it[3] == '-';
}

void uva::xml::ignore_comment(const char*& it, const char* end)
{
    it += 3;

    bool found_end_comment = false;

    while(it != end && !found_end_comment) {
        switch (*it)
        {
        case '-':
            if((end - it) >= 3) {
                ++it;

                if(*it == '-') {
                    ++it;

                    if(*it == '>') {
                        ++it;
                        found_end_comment = true;
                    }
                }
            }
            break;
        
        default:
            ++it;
            break;
        }
    }

    if(!found_end_comment) {
        throw std::runtime_error("expected end of comment");
    }
}

uva::xml uva::xml::decode(std::string_view xml_source)
{
    uva::xml xml;
    xml.source_view = std::string_view(xml_source);
    
    const char* start = xml_source.data();
    const char* it    = xml_source.data();
    const char* end   = xml_source.end();

    auto advance_iterator = [&]() {
        while(it != end && isspace(*it)) {
            ++it;
        }
    };

    advance_iterator();

tag_begin:

    if(it == end) {
        throw std::runtime_error("unexpected end of file");
    }

    if(*it != '<') {
        throw std::runtime_error("expected '<'");
    }

    // Check for comment
    if(is_comment(it, end)) {

        ignore_comment(it, end);
        advance_iterator();

        goto tag_begin;
    }

    ++it;

    advance_iterator();

    if(it == end) {
        throw std::runtime_error("unexpected end of file");
    }

    bool is_question_mark_tag = *it == '?';
    bool expect_close_tag = true;

    if(is_question_mark_tag) {
        ++it;
    }

    advance_iterator();

    const char* tag_start = it;

    while(it != end && !isspace(*it) && (isalnum(*it) || *it == '-' || *it == '_' || *it == ':')) {
        ++it;
    }

    size_t tag_size = it - tag_start;

    xml.tag = std::string_view(tag_start, tag_size);

    advance_iterator();

    if(it == end) {
        throw std::runtime_error("unexpected end of file");
    }

    advance_iterator();

    if(it == end) {
        throw std::runtime_error("unexpected end of file");
    }

    switch (*it)
    {
    case '>':
        break;
    case '/':
        expect_close_tag = false;
        ++it;

        if(it == end) {
            throw std::runtime_error("unexpected end of file");
        }
        break;
    default:
        while(end - it) {

            const char* key_start = it;

            if(!isalpha(*it)) {
                throw std::runtime_error("expected attribute key");
            }

            while(end != it && !isspace(*it) && *it != '=') {
                it++;
            }

            const char* key_end = it;

            advance_iterator();

            if(it == end) {
                throw std::runtime_error("unexpected end of file");
            }

            if(*it != '=') {
                throw std::runtime_error("expected '='");
            }

            ++it;

            advance_iterator();

            if(it == end) {
                throw std::runtime_error("unexpected end of file");
            }

            if(*it != '"') {
                throw std::runtime_error("expected '\"'");
            }

            ++it;

            if(it == end) {
                throw std::runtime_error("unexpected end of file");
            }

            const char* value_start = it;

            while(end != it && *it != '"') {
                it++;
            }

            if(it == end) {
                throw std::runtime_error("unexpected end of file");
            }

            std::string_view key(key_start, key_end - key_start);
            std::string_view value(value_start, it - value_start);
            xml.attributes[uva::xml::string(key)] = uva::xml::string(value);

            ++it;
            advance_iterator();

            if(it == end) {
                throw std::runtime_error("unexpected end of file");
            }

            if(is_question_mark_tag) {
                if(*it == '?') {
                    ++it;
                    advance_iterator();
                    break;
                }
            } else {
                if(*it == '>') {
                    break;
                }
                if(*it == '/') {
                    ++it;
                    advance_iterator();
                    expect_close_tag = false;
                    break;
                }
            }
        }
        break;
    }

    if(*it != '>') {
       throw std::runtime_error("expected '>'");
    }

    ++it;

    if(is_question_mark_tag) {
        std::string_view actual_xml_view(it, end - it);
        uva::xml actual_xml = uva::xml::decode(actual_xml_view);
        actual_xml.source = std::move(xml.source);

        if(xml.tag == "xml") {
            actual_xml.declaration = std::make_shared<uva::xml>(std::move(xml));
        } else if(xml.tag == "xml-model") {
            actual_xml.model = std::make_shared<uva::xml>(std::move(xml));
        }
        
        return actual_xml;
    }

    if(expect_close_tag) {

        advance_iterator();

        if(end - it > 2) {
            if(*it == '<') {
                while(end != it && *(uint16_t*)it != *(uint16_t*)"</") {
                    if(is_comment(it, end)) {
                        ignore_comment(it, end);
                        advance_iterator();
                    } else {                        
                        std::string_view xml_view(it, end - it);
                        uva::xml child = uva::xml::decode(xml_view);

                        xml.childrens.push_back(std::move(child));

                        it = child.source_view.end();

                        advance_iterator();
                    }
                }
            } else {
                const char* content_start = it;
                while(end != it && *it != '<') {
                    it++;
                }
                xml.content = std::string_view(content_start, it - content_start);
            }
        }

        advance_iterator();

        if(it == end) {
            throw std::runtime_error("unexpected end of file");
        }

        if(*it != '<') {
            throw std::runtime_error("expected '<'");
        }

        ++it;

        if(it == end) {
            throw std::runtime_error("unexpected end of file");
        }

        if(*it != '/') {
            throw std::runtime_error("expected '/'");
        }

        ++it;

        advance_iterator();

        if(it == end) {
            throw std::runtime_error("unexpected end of file");
        }

        const char* end_tag_start = it;

        if(end - it < xml.tag.size()) {
            throw std::runtime_error("unexpected end of file");
        }

        if(memcmp(it, xml.tag.data(), xml.tag.size()) != 0) {
            throw std::runtime_error("expected end tag to match start tag");
        }

        it+= xml.tag.size();

        advance_iterator();

        if(it == end) {
            throw std::runtime_error("unexpected end of file");
        }

        if(*it != '>') {
            throw std::runtime_error("expected '>'");
        }

        ++it;
    }
    
    xml.source_view = std::string_view(start, it - start);
    
    return xml;
}

uva::xml::string uva::xml::try_attribute(uva::xml::string key)
{
    auto it = attributes.find(key);

    if(it == attributes.end()) {
        return "";
    }

    return it->second;
}

uva::xml::string uva::xml::attribute(uva::xml::string key, uva::xml::string __default) const
{
    auto it = attributes.find(key);

    if(it == attributes.end()) {
        return __default;
    }

    return it->second;
}

bool uva::xml::enumerate_attribute(string key, const uva::enumeration& e, size_t& value, const std::string_view** actual) const
{
    auto it = attributes.find(key);

    if(it == attributes.end()) {
        return false;
    }

    if(actual) {
        *actual = &it->second;
    }

    if(it->second.empty()) {
        return false;
    }

    bool is_all_digits = true;

    for(const char& c : it->second) {
        if(!isdigit(c)) {
            is_all_digits = false;
            break;
        }
    }

    if(is_all_digits) {
        value = atoi(it->second.data());
        return true;
    }

    auto enum_it = e.values.find(it->second);

    if(enum_it == e.values.end()) {
        return false;
    }

    value = enum_it->second;

    return true;
}