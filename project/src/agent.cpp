#include "agent.h"

#include <print>
#include <string>
#include <vector>

#include "agent_rules.h"
#include "event.h"
#include "event_list.h"
#include "rules.h"

namespace nano_edr{

Agent::Agent(bool quiet): quiet_(quiet), engine_(BuildAgentRules()){}

void Agent::HandleEvent(const Event& event){
    ++events_;
    Typecnt(event.type());

    std::vector<const IRule*> fired;
    engine_.ProcessEvent(event, &fired);
    if (!fired.empty() && !quiet_) {
        Context();
    }
    for (auto rule : fired){
        std::print("[DETECT] {}  {}  ts={} pid={}\n", SeverityName(rule->severity()), rule->id(), event.raw_ts(), event.pid());
    }

    WindowEntry entry;
    entry.ts = event.ts();
    entry.type = event.type();
    entry.pid = event.pid();
    window_.PushBack(entry);
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
    const std::size_t n = window_.size();
    if (n == 0){
        return;
    }
    if (n >= 2){
        const WindowEntry& prev = window_[n - 2];
        std::print("[CTX] -2: {} {} pid={}\n", prev.ts.ms, prev.type, prev.pid);
    }
    const WindowEntry& last = window_.back();
    std::print("[CTX] -1: {} {} pid={}\n", last.ts.ms, last.type, last.pid);
}
void Agent::PrintSummary() const {
    if (!quiet_) {
        std::print("событий {}, пропущено {}, битых {}\n", events_, skipped_, broken_);
        for (std::size_t j = 0; j < type_names_.size(); ++j) {
            std::print("  {}: {}\n", type_names_[j], type_counts_[j]);
        }
        std::print("[STAT] в окне {} из {} мест\n", window_.size(), window_.capacity());
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