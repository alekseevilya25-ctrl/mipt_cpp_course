#include "parse.h"

namespace nano_edr {
const char* ws = " \t";

bool  IsBlankOrComment(const std::string* line) {
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
    if (IsBlankOrComment (line)) return false;

    std::vector <Field> fields;
    std::string ts;    
    std::string type;
    std::string pid;

    bool ts_ok = false;
    bool type_ok = false;
    bool pid_ok = false;

    size_t n = (*line).size();

    std::size_t i = 0;

    const std::string& s = (*line);

    while (i < n){
        i = s.find_first_not_of(ws, i);
        if (i == std::string::npos) {
            break;
        }
        size_t start = i;
        while (s[i] != '=' && s[i] != ' ' && i < n &&  s[i] != '\t'){
            i++;
        }
        if (i == n || s[i] != '=' || i == start){
            return false;
        }

        std::string key = s.substr(start, i - start);
        std::string value = "";
        i++;

        if (i < n && s[i] == '"' ){
            i++;
            start = i;
            while (i < n && s[i] != '"' ){
                i++;
            }
            if (i == n){
                return false;
            }

            value = s.substr(start, i - start);
            i++;
            if (i < n && s[i] != ' ' && s[i] != '\t'){
                return false;
            }
        }
        else{
            if(i < n){
                start = i;
                while(i < n && s[i] !=' ' && s[i] != '\t'){
                    i++;
                }
                value = s.substr(start, i - start);
            }
        }
        if(key == "ts" && !ts_ok){
            ts = value;
            ts_ok = true;
            continue;
        }
        if(key == "type" && !type_ok){
            type = value;
            type_ok = true;
            continue;
        }
        if(key == "pid" && !pid_ok){
            pid = value;
            pid_ok = true;
            continue;
        }
        Field k;
        k.key = key;
        k.value = value;
        fields.push_back(k);
    }
    if(!ts_ok || !type_ok){
        return false;
    }
    out -> fields = fields;
    out -> ts = ts;
    out -> type = type;
    out -> pid = pid;
    return true;
}
}