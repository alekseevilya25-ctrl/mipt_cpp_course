#ifndef NANO_EDR_AGENT_H
#define NANO_EDR_AGENT_H

#include <cstddef>
#include <string>
#include <vector>

#include "event.h"
#include "event_list.h"
#include "event_source.h"
#include "rule.h"
#include "rule_engine.h"

namespace nano_edr{

class Agent : public IEventHandler {
public:
    Agent(std::size_t window_size, bool quiet);

    void PrintSummary() const;

    // Оба источника: пришло событие. Тело — как на шаге 6.
    void HandleEvent(const Event& event) override;

    // FileSource: пустая строка или комментарий.
    void OnSkipped() override;

    // FileSource: из строки не вышло события.
    // Трамплин: событие от os.h не удалось построить.
    void OnBroken() override;

    // OsSource: Wait вернул OS_TIMEOUT, новых событий пока нет.
    void OnTick() override;
    // ...
private:
    void Typecnt(const std::string& type);

    void Context() const;

    bool quiet_ = false;
    long long events_ = 0;
    std::vector<std::string> type_names_;
    std::vector<long long> type_counts_;
    EventList window_;
    RuleEngine engine_;
    long long skipped_ = 0;
    long long broken_ = 0;
    const EventNode* ctx_ = nullptr;
};

}
#endif  // NANO_EDR_AGENT_H