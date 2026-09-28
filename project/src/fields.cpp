#include "fields.h"
#include <stdexcept>
#include <charconv>
namespace nano_edr{
// nullptr, а не исключение: у file_write нет поля domain, и спрашивать
// про него законно.
const std::string* FindField(const Event& event, const std::string& key){
    for (auto& field: event.fields){
        if (field.key == key){
            return &field.value;
        }
    }
    return nullptr;
}
//throw std::invalid_argument: отсутствие обязательного поля
//без которого дальше идти некуда
const std::string& GetRequiredField(const Event& event, const std::string& key){
    const std::string* value = FindField(event, key);
    if (!value){
        throw std::invalid_argument("поле отсутствует: " + key);
    }
    return *value;
}
//false при битых данных и переполнении "12.5", "123abc", "99999999999999999999999"
bool GetIntField(const Event& event, const std::string& key, uint64_t* out){
    const std::string* s = FindField(event, key);
    if (!s) return false;

    uint64_t n = 0;
    auto [ptr, ec] = std::from_chars(s->data(), s->data() + s->size(), n);
    if (ec != std::errc{} || ptr != s->data() + s->size()) {
        return false;
    }

    *out = n;
    return true;
}
uint64_t GetIntField(const Event& event, const std::string& key, uint64_t fallback){
    uint64_t out;
    if (!GetIntField(event, key, &out)){
        return fallback;
    }
    return out;
}
bool IsProcessStart(const Event& event){
    if (event.type == "process_start") return true;
    return false;
}
bool IsFileWrite(const Event& event){
    if (event.type == "file_write") return true;
    return false;
}
bool IsNetConnect(const Event& event){
    if (event.type == "net_connect") return true;
    return false;
}
std::string NormalizePath(const std::string& path){
    std::string str = path;

    for (char& c : str){
        if (c >= 'A' && c <='Z'){
            c += 32;
        }
    }
    for (char& c : str){
        if (c == '/'){
            c = '\\';
        }
    }
    size_t pos = str.find("%temp%");
    while (pos != std::string::npos){
        str.replace(pos, 6, "\\appdata\\local\\temp");
        pos = str.find("%temp%");
    }
    pos = str.find("%tmp%");
    while (pos != std::string::npos){
        str.replace(pos, 5, "\\appdata\\local\\temp");
        pos = str.find("%tmp%");
    }
    std::string norm;
    size_t n = str.size();
    for (size_t i = 0; i < n; i++){
        if(i < n - 1 && str[i] == '\\' && str[i + 1] == '\\'){
            continue;
        }
        norm += str[i];
    }
    return norm;
}
bool PathEndsWith(const Event& event, const std::string& suffix){
    const std::string* path = FindField(event, "path");
    if (path == nullptr){
        return false;
    }
    std::string npath = NormalizePath(*path);
    std::string nsuffix = NormalizePath(suffix);
    if (nsuffix.size() > npath.size()){
        return false;
    }
    size_t pos = npath.size() - nsuffix.size();
    if (npath.find(nsuffix, pos) == pos){
       return true;
    }
    return false;
}

bool CommandLineContains(const Event& event, const std::string& needle){
    const std::string* cmd = FindField(event, "cmdline");
    if (!cmd){
        return false;
    }
    std::string cmd_ = NormalizePath(*cmd);
    std::string needle_ = NormalizePath(needle);
    if (cmd_.find(needle_) != std::string::npos){
        return true;
    }
    else{
        return false;
    }
}
}