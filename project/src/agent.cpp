#include "agent.h"

#include <print>
#include <string>
#include <vector>
#include <utility>

#include "agent_rules.h"
#include "event.h"
#include "event_list.h"
#include "rules.h"
#include "dedup_sink.h"

namespace nano_edr{

Agent::Agent(bool quiet, uint64_t dedup_period_ms): quiet_(quiet), engine_(BuildAgentRules(&model_)), dedup_period_ms_(dedup_period_ms){
    engine_.SetModel(&model_);
}

void Agent::HandleEvent(const Event& event){
    ++events_;
    Typecnt(event.type());
    context_printed_ = false;
    std::vector<const IRule*> fired;
    model_.Observe(event);
    if (responder_ != nullptr && responder_->Recognize(event)) {
        WindowEntry entry;
        entry.ts = event.ts();
        entry.type = event.type();
        entry.pid = event.pid();
        window_.PushBack(std::move(entry));
        return;
    }
    engine_.ProcessEvent(event, &fired);
    WindowEntry entry;
    entry.ts = event.ts();
    entry.type = event.type();
    entry.pid = event.pid();
    window_.PushBack(std::move(entry));
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

void Agent::PrintContextOnce() {
    if (quiet_) return;
    if (context_printed_) return;
    context_printed_ = true;
    Context();
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
        std::print("[STAT] в модели процессов {}, файлов {}\n", model_.process_count(), model_.file_count());
        std::print("[STAT] подавлено дубликатов {} (срок {} мс)\n", suppressed_, dedup_period_ms_);
        if (responder_ != nullptr) {
            const auto& s = responder_->stats();
            std::print("[STAT] действий запланировано {}, повторов отброшено {}\n", s.planned, s.skipped_repeat);
            std::print("[STAT] завершено процессов {}, изолировано файлов {}, удалено файлов {}\n", s.killed, s.quarantined, s.deleted);
        if (s.own_events > 0) {
            std::print("[STAT] своих событий {}\n", s.own_events);
        }
}
        std::print("\nправила, детектов:\n");
        for (std::size_t j = 0; j < engine_.size(); ++j) {
            std::print("{} {}\n", engine_.rule(j).id(), engine_.rule(j).hits());
        }
    }
}

void Agent::Subscribe(AlertSink sink) {
    FanOutSink all;
    all.Add([this](const Detection&) { PrintContextOnce(); });
    all.Add(std::move(sink));
    engine_.Subscribe(DedupSink(std::move(all), dedup_period_ms_, &suppressed_));
}

void Agent::OnSkipped(){
    ++skipped_;
}
void Agent::OnBroken(){
    ++broken_;
}
void Agent::OnTick(){
}

void Agent::EnableResponse(OsHandle* os, bool dry_run, const std::string& quarantine_dir) {
    planner_ = std::make_unique<ResponsePlanner>(&model_, quarantine_dir);
    responder_ = std::make_unique<Responder>(os, dry_run);
    engine_.Subscribe([this](const Detection& detection) {
        responder_->Execute(planner_->Plan(detection));
    });
}

}