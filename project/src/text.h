#ifndef NANO_EDR_TEXT_H
#define NANO_EDR_TEXT_H

#include <string>

namespace nano_edr{

bool TextEquals(const std::string& text, const std::string& value);

bool TextContains(const std::string& text, const std::string& value);

bool TextEndsWith(const std::string& text, const std::string& value);

}

#endif