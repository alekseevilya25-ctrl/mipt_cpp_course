#include "conditions.h"
#include "fields.h"
#include "text.h"

#include <algorithm>
#include <vector>


namespace nano_edr{

bool FieldEquals::Matches(const Event& event) const {
    const std::string* value = FindField(event, key_);
    if (value == nullptr){
        return false;
    }
    return TextEquals(*value, value_);
}
bool FieldContains::Matches(const Event& event) const {
    const std::string* value = FindField(event, key_);
    if (value == nullptr){
        return false;
    }
    return TextContains(*value, needle_);
}
bool FieldEndsWith::Matches(const Event& event) const {
    const std::string* value = FindField(event, key_);
    if (value == nullptr){
        return false;
    }
    return TextEndsWith(*value, suffix_);
}
bool FieldInList::Matches(const Event& event) const {
    const std::string* value = FindField(event, key_);
    if (value == nullptr){
        return false;
    }
    for (const auto& val : values_){
        if (TextEquals(*value, val)){
            return true;
        }
    }
    return false;
}
bool EventTypeIs::Matches(const Event& event) const {
    const EventType type = event.event_type();
    for (const auto& type_ : types_){
        if (type == type_){
            return true;
        }
    }
    return false;
}

void AllOf::Add(std::unique_ptr<ICondition> condition){
    parts_.push_back(std::move(condition));
}
bool AllOf::Matches(const Event& event) const {
    for (const auto& part : parts_){
        if (!part->Matches(event)){
            return false;
        }
    }
    return true;
}

void AnyOf::Add(std::unique_ptr<ICondition> condition) {
    parts_.push_back(std::move(condition));
}
bool AnyOf::Matches(const Event& event) const {
    for (const auto& part : parts_){
        if (part->Matches(event)){
            return true;
        }
    }
    return false;
}

}
