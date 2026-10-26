#include "agent.h"

#include <print>
#include <string>
#include <vector>

#include "agent_rules.h"
#include "event.h"
#include "event_list.h"
#include "rules.h"

namespace nano_edr{

Agent::Agent(std::size_t window_size, bool quiet): quiet_(quiet), window_(window_size), engine_(BuildAgentRules()){}

void Agent::HandleEvent(const Event& event){
    ++events_;
    bool detect = false;
    Typecnt(event.type());

    std::vector<const IRule*> fired;
    engine_.ProcessEvent(event, &fired);
    for (auto rule : fired){
        std::print("[DETECT] {}  {}  ts={} pid={}\n", SeverityName(rule->severity()), rule->id(), event.raw_ts(), event.pid());
        detect = true;
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
        std::print("событий {}, пропущено {}, битых {}\n", events_, skipped_, broken_);
        for (std::size_t j = 0; j < type_names_.size(); ++j) {
            std::print("  {}: {}\n", type_names_[j], type_counts_[j]);
        }
        std::print("\nправила, детектов:\n");
        for (std::size_t j = 0; j < engine_.size(); ++j) {
            std::print("{} {}\n", engine_.rule(j).id(), engine_.rule(j).hits());
        }
    }
}

void Agent::OnSkipped(){
    ++skipped_;
}
void Agent::OnBroken(){
    ++broken_;
}
void Agent::OnTick(){
}
}