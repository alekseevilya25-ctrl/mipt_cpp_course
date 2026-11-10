#include "rule.h"
#include <print>
namespace nano_edr{

RuleBase::RuleBase(const std::string& id, Severity severity): id_(id), severity_(severity){}

void RuleBase::AddAction(const std::string& action){
    actions_.push_back(action);
}

MatchRule::MatchRule(const std::string& id, Severity severity): RuleBase(id, severity) {}

void MatchRule::AddCondition(std::unique_ptr<ICondition> condition){
    conditions_.push_back(std::move(condition));
}

bool MatchRule::Check(const Event& event){
    for (auto& condition : conditions_){
        if(!condition->Matches(event)){
            return false;
        }
    }
    CountHit();
    return true;
}

SequenceRule::SequenceRule(const std::string& id, Severity severity, uint64_t window_ms): RuleBase(id, severity), window_ms_(window_ms){}

void SequenceRule::SetFirst(std::unique_ptr<ICondition> condition){
    first_ = std::move(condition);
}

void SequenceRule::SetSecond(std::unique_ptr<ICondition> condition){
    second_ = std::move(condition);
}

bool SequenceRule::Linked(const Pending& first, const Event& second) const {
    if (child_of_prev_){
        if (model_ == nullptr){
            return false;
        }
        const ProcessRecord* record = model_->FindProcess(second.pid());
        if (record == nullptr){
            return false;
        }
        return record->ppid == first.pid;
    }
    if (same_pid_){
        return first.pid == second.pid();
    }
    return true;
}

bool SequenceRule::Check(const Event& event){
    if (!first_ || !second_){
        return false;
    }
    std::vector<Pending> newpend;
    for (const auto& p : pending_){
        if (event.ts().ms - p.ts_ms <= window_ms_){
            newpend.push_back(p);
        }
    }
    pending_ = std::move(newpend);
    if (second_->Matches(event)){
        for (std::size_t i = 0; i < pending(); i++){
            if (Linked(pending_[i], event)){
                pending_.erase(pending_.begin() + i);
                CountHit();
                return true;
            }
        }
    }
    if (first_->Matches(event)){
        Pending p;
        p.ts_ms = event.ts().ms;
        p.pid = event.pid();
        pending_.push_back(p);
    }
    return false;
}

LambdaRule::LambdaRule(const std::string& id, Severity severity, Predicate predicate): RuleBase(id, severity), predicate_(std::move(predicate)) {}

bool LambdaRule::Check(const Event& event) {
    if (!predicate_) {
        return false;
    }
    if (!predicate_(event)) {
        return false;
    }
    CountHit();
    return true;
}
}