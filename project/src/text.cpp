#include "text.h"

namespace nano_edr{
std::string ToLower(const std::string& s) {
    std::string result = s;
    for (char& c : result) {
        if (c >= 'A' && c <='Z'){
            c += 32;
        }
    }
    return result;
}
bool TextEquals(const std::string& text, const std::string& value){
    return ToLower(text) == ToLower(value);
}

bool TextContains(const std::string& text, const std::string& value){
    return ToLower(text).find(ToLower(value)) != std::string::npos;
}

bool TextEndsWith(const std::string& text, const std::string& value){
    if (value.size() > text.size()) {
        return false;
    }
    size_t pos = text.size() - value.size();
    return ToLower(text).find(ToLower(value), pos) == pos;
}

}