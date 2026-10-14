#ifndef NANO_EDR_AGENT_H
#define NANO_EDR_AGENT_H

#include <cstddef>
#include <string>
#include <vector>

#include "os.h"
#include "event_list.h"
#include "rule.h"

class Agent {
public:
    Agent(std::size_t window_size, bool quiet);

    void HandleEvent(const nano_edr::Event& event);

    static void Trampoline(const os_event* ev, void* ctx) noexcept;

    void Typecnt(const std::string& type);

    void PrintSummary() const;

    void IncComments();

    void IncLines();

private:
    bool quiet_ = false;
    long long lines_ = 0;
    long long events_ = 0;
    long long comments_ = 0;
    std::vector<std::string> type_names_;
    std::vector<long long> type_counts_;
    nano_edr::EventList window_;
    std::vector<nano_edr::MatchRule> rules_;
};

#endif  // NANO_EDR_AGENT_H