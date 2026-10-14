#include "agent.h"

#include <print>
#include <string>
#include <vector>

#include "agent_rules.h"
#include "event.h"
#include "event_list.h"
#include "rules.h"

Agent::Agent(std::size_t window_size, bool quiet): window_(window_size), quiet_(quiet), rules_(nano_edr::BuildAgentRules()){}

void Agent::HandleEvent(const nano_edr::Event& event){
    ++events_;
    Typecnt(event.type());
    for (auto& rule : rules_){
        if(rule.Check(event)){
            std::print("[DETECT] {}  {}  ts={} pid={}\n", SeverityName(rule.severity()), rule.id(), event.raw_ts(), event.pid());
        }
    }
    window_.PushBack(event);
}

void Agent::Trampoline(const os_event* ev, void* ctx) noexcept{
    try {
        nano_edr::EventParts parts;
        for (size_t i = 0; i < ev->field_count; i++){
            nano_edr::Field k;
            k.key = ev->fields[i].key;
            k.value = ev->fields[i].value;
            parts.fields.push_back(k);
        }
        parts.ts = std::to_string(ev->ts);
        parts.type = ev->type;
        parts.pid = ev->pid == 0 ? "" : std::to_string(ev->pid);
        static_cast<Agent*>(ctx)->HandleEvent(nano_edr::Event(parts));
    }
    catch (std::exception& e){
    }
}

void Agent::Typecnt(const std::string& type){
    for (std::size_t i = 0; i < type_names_.size(); ++i){
        if (type_names_[i] == type) {
            ++type_counts_[i];
            return;
        }
    }
    type_names_.push_back(type);
    type_counts_.push_back(1);
}

void Agent::PrintSummary() const {
    if (!quiet_) {
        std::print("строк {}, из них комментариев {}, событий {}\n", lines_, comments_, events_);
        for (std::size_t j = 0; j < type_names_.size(); ++j) {
            std::print("  {}: {}\n", type_names_[j], type_counts_[j]);
        }
        std::print("\nправила, детектов:\n");
        for (const auto& rule : rules_) {
            std::print("{} {}\n", rule.id(), rule.hits());
        }
    }
}

void Agent::IncComments(){
    comments_++;
}

void Agent::IncLines(){
    lines_++;
}