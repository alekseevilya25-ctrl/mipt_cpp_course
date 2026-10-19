#ifndef NANO_EDR_AGENT_H
#define NANO_EDR_AGENT_H

#include <cstddef>
#include <string>
#include <vector>

#include "os.h"
#include "event_list.h"
#include "rule.h"

namespace nano_edr{

class Agent {
public:
    Agent(std::size_t window_size, bool quiet);

    void HandleEvent(const Event& event);

    static void Trampoline(const os_event* ev, void* ctx) noexcept;

    void PrintSummary() const;

private:
    void Typecnt(const std::string& type);

    void Context() const;

    bool quiet_ = false;
    long long events_ = 0;
    std::vector<std::string> type_names_;
    std::vector<long long> type_counts_;
    EventList window_;
    std::vector<MatchRule> rules_;
    const EventNode* ctx_ = nullptr;
};

}
#endif  // NANO_EDR_AGENT_H