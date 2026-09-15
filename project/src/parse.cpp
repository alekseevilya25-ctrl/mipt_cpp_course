#include "parse.h"

namespace nano_edr {
const char* ws = " \t";

bool  IsBlanckOrComment(const std::string* line) {
    if (line->empty()){
        return true;
    }

    size_t pos = line -> find_first_not_of(ws);
    if (pos == std::string::npos) {
        return true;
    }

    return (*line)[pos] == '#' || (*line)[pos] == ';';
}
bool ParseEventLine(const std::string* line, Event* out){
    
}
}