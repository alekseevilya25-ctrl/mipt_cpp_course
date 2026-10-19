#include "agent.h"

#include <print>
#include <string>
#include <vector>

#include "agent_rules.h"
#include "event.h"
#include "event_list.h"
#include "rules.h"

namespace nano_edr{

Agent::Agent(std::size_t window_size, bool quiet): quiet_(quiet), window_(window_size), rules_(BuildAgentRules()){}

void Agent::HandleEvent(const Event& event){
    ++events_;
    bool detect = false;
    Typecnt(event.type());
    for (auto& rule : rules_){
        if(rule.Check(event)){
            std::print("[DETECT] {}  {}  ts={} pid={}\n", SeverityName(rule.severity()), rule.id(), event.raw_ts(), event.pid());
            detect = true;
        }
    }
    if (detect && !quiet_) {
        Context();
    }
    window_.PushBack(event);

    if (window_.size() <= 2) {
        ctx_ = window_.head();
    } else {
        ctx_ = ctx_->next;
    }
}

void Agent::Trampoline(const os_event* ev, void* ctx) noexcept{
    try {
        EventParts parts;
        for (size_t i = 0; i < ev->field_count; i++){
            Field k;
            k.key = ev->fields[i].key;
            k.value = ev->fields[i].value;
            parts.fields.push_back(k);
        }
        parts.ts = std::to_string(ev->ts);
        parts.type = ev->type;
        parts.pid = ev->pid == 0 ? "" : std::to_string(ev->pid);
        static_cast<Agent*>(ctx)->HandleEvent(Event(parts));
    }
    catch (const std::exception& e) {
        std::print(stderr, "Trampoline: {}\n", e.what());
    }
    catch (...) {
        std::print(stderr, "Trampoline: неизвестная ошибка\n");
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
void Agent::Context() const {
    const EventNode* node = ctx_;
    if (node == nullptr){
        return;
    }
    int i = 2;
    const EventNode* tmp = node;
    int count = 0;
    while (tmp != nullptr){
        ++count;
        tmp = tmp->next;
    }
    i = count;
    while (node != nullptr){
        std::print("[CTX] -{}: ts={}  type={} pid={}\n", i, node->event.raw_ts(), node->event.type(), node->event.pid());
        --i;
        node = node->next;
    }
}
void Agent::PrintSummary() const {
    if (!quiet_) {
        std::print("событий {}\n", events_);
        for (std::size_t j = 0; j < type_names_.size(); ++j) {
            std::print("  {}: {}\n", type_names_[j], type_counts_[j]);
        }
        std::print("\nправила, детектов:\n");
        for (const auto& rule : rules_) {
            std::print("{} {}\n", rule.id(), rule.hits());
        }
    }
}

}