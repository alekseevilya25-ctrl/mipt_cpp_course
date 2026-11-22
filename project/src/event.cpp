#include "event.h"
#include "copy_stats.h"

#include <charconv>
#include <stdexcept>
#include <utility>
namespace nano_edr {

Timestamp Event::Validate(const std::string& raw_ts, const std::string& type){
    Timestamp ts;
    auto [ptr, ec] = std::from_chars(
        raw_ts.data(), raw_ts.data() + raw_ts.size(), ts.ms);
    if (ec != std::errc{} || ptr != raw_ts.data() + raw_ts.size()){
        throw std::invalid_argument("неверное время события: " + raw_ts);
    }
    if (type.empty()){
        throw std::invalid_argument("отсутствует тип события");
    }
    return ts;
}

Event::Event(const EventParts& parts): ts_(Validate(parts.ts, parts.type)),
    raw_ts_(parts.ts),
    type_(parts.type),
    event_type_(EventTypeFromString(parts.type)),
    pid_(parts.pid),
    fields_(parts.fields){
    copy_stats().event_strings_copied += 3 + 2 * parts.fields.size();
}

Event::Event(EventParts&& parts): ts_(Validate(parts.ts, parts.type)), event_type_(EventTypeFromString(parts.type)){
    copy_stats().event_strings_moved += 3 + 2 * parts.fields.size();
    raw_ts_ = std::move(parts.ts);
    type_ = std::move(parts.type);
    pid_ = std::move(parts.pid);
    fields_ = std::move(parts.fields);
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