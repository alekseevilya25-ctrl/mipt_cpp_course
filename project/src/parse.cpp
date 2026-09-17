#include "parse.h"

namespace nano_edr {
const char* ws = " \t";

bool Valid (const std::string& s, size_t n, size_t i){
    return (i < n && s[i] !=' ' && s[i] != '\t');
}


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

    std::vector <Field>& fields = out->fields;
    std::string& ts = out->ts;    
    std::string& type = out->type;
    std::string& pid = out->pid;

    bool ts_ok = false;
    bool type_ok = false;
    bool pid_ok = false;

    size_t n = (*line).size();

    size_t i = 0;

    const std::string& s = (*line);

    while (i < n){
        i = s.find_first_not_of(ws, i);
        if (i == std::string::npos) {
            break;
        }
        size_t start = i;
        while (Valid(s, n, i) && (s[i] != '=')){
            i++;
        }
        if (i == n || s[i] != '=' || i == start){
            return false;
        }

        std::string key = s.substr(start, i - start);
        std::string value;
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
            if (Valid(s, n, i)){
                return false;
            }
        }
        else{
            if(i < n){
                start = i;
                while(Valid(s, n, i)){
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
    if (ts.empty() || type.empty()){
        return false;
    }
    return true;
}
}