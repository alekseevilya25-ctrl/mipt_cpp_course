#include "event.h"

#include <charconv>
#include <stdexcept>
namespace nano_edr {

Event::Event(const EventParts& parts): raw_ts_(parts.ts), type_(parts.type), pid_(parts.pid), fields_(parts.fields) {
    const std::string& ts = parts.ts;
    uint64_t n = 0;
    auto [ptr, ec] = std::from_chars(ts.data(), ts.data() + ts.size(), n);
    if (ec != std::errc{} || ptr != ts.data() + ts.size()) {
        throw std::invalid_argument("неверное время события: " + ts);
    }
    if(parts.type.empty()){
        throw std::invalid_argument("отсутствует тип события");
    }
    ts_.ms = n;
}

std::string ToString(const Event& event){
    std::string str;
    str += "ts=" + event.raw_ts() + " ";
    str += "type=" + event.type();
    if(!event.pid().empty()){
        str += " pid=" + event.pid();
    }
    for (auto& field : event.fields()){
        str += " " + field.key + "=" + field.value;
    }
    return str;
}


std::ostream& operator<<(std::ostream& out, const Event& event){
    return out << ToString(event);
}
}