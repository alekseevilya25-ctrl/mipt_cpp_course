#include "field_traits.h"
#include "text.h"

namespace nano_edr {

bool FieldTraits<std::string>::Parse(const std::string& text, std::string* out) {
    *out = text;
    return true;
}

bool FieldTraits<std::uint64_t>::Parse(const std::string& text, std::uint64_t* out) {
    if (text.empty()){
        return false;
    }
    std::uint64_t value = 0;
    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (ec != std::errc{} || ptr != text.data() + text.size()){
        return false;
    }
    *out = value;
    return true;
}

bool FieldTraits<bool>::Parse(const std::string& text, bool* out) {
    if (TextEquals(text, "1") || TextEquals(text, "true") || TextEquals(text, "yes")){
        *out = true;
        return true;
    }
    if (TextEquals(text, "0") || TextEquals(text, "false") || TextEquals(text, "no")){
        *out = false;
        return true;
    }
    return false;
}

}  // namespace nano_edr
