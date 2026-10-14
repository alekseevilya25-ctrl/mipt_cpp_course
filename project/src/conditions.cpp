#include "conditions.h"
#include "fields.h"
#include "text.h"

#include <vector>

namespace nano_edr{

bool FieldEquals::operator()(const Event& event) const {
    const std::string* value = FindField(event, key_);
    if (value == nullptr){
        return false;
    }
    return TextEquals(*value, value_);
}
bool FieldContains::operator()(const Event& event) const {
    const std::string* value = FindField(event, key_);
    if (value == nullptr){
        return false;
    }
    return TextContains(*value, needle_);
}
bool FieldEndsWith::operator()(const Event& event) const {
    const std::string* value = FindField(event, key_);
    if (value == nullptr){
        return false;
    }
    return TextEndsWith(*value, suffix_);
}
bool FieldInList::operator()(const Event& event) const {
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
Condition Condition::Equals(const std::string& key, const std::vector<std::string>& values){
    return Condition(Match::kEquals, key, values);
}
Condition Condition::Contains(const std::string& key, const std::vector<std::string>& values){
    return Condition(Match::kContains, key, values);
}
Condition Condition::EndsWith(const std::string& key, const std::vector<std::string>& values){
    return Condition(Match::kEndsWith, key, values);
}
bool Condition::operator()(const Event& event) const {
    const std::string* value = FindField(event, key_);
    if (value == nullptr || values_.empty()){
        return false;
    }
    switch (match_){
    case Match::kEquals:
        for (const auto& v : values_){
            if (TextEquals(*value, v)){
                return true;
            }
        }
        return false;
    case Match::kContains:
        for (const auto& v : values_){
            if (TextContains(*value, v)){
                return true;
            }
        }
        return false;
    case Match::kEndsWith:
        for (const auto& v : values_){
            if (TextEndsWith(*value, v)){
                return true;
            }
        }
        return false;
    }
    return false;
}
}
