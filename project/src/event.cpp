#include "event.h"

#include <charconv>
#include <stdexcept>
namespace nano_edr {

Event::Event(const EventParts& parts): raw_ts_(parts.ts), type_(parts.type), pid_(parts.pid), fields_(parts.fields), event_type_(EventTypeFromString(parts.type)) {
    const std::string& ts = parts.ts;
    auto [ptr, ec] = std::from_chars(ts.data(), ts.data() + ts.size(), ts_.ms);
    if (ec != std::errc{} || ptr != ts.data() + ts.size()) {
        throw std::invalid_argument("неверное время события: " + ts);
    }
    if(parts.type.empty()){
        throw std::invalid_argument("отсутствует тип события");
    }
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

EventType EventTypeFromString(const std::string& type){
    if(type == "process_start") return EventType::kProcessStart;
    if(type == "process_end") return EventType::kProcessEnd;
    if(type == "file_create") return EventType::kFileCreate;
    if(type == "file_write") return EventType::kFileWrite;
    if(type == "file_delete") return EventType::kFileDelete;
    if(type == "file_move") return EventType::kFileMove;
    if(type == "net_connect") return EventType::kNetConnect;
    return EventType::kOther;
}
}