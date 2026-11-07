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
#include "ring_buffer.h"

namespace nano_edr{

struct WindowEntry {
    Timestamp ts;
    std::string type;
    std::string pid;
};

class Agent : public IEventHandler {
public:
    explicit Agent(bool quiet);

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
    static constexpr std::size_t kWindowCapacity = 256;

private:
    void Typecnt(const std::string& type);

    void Context() const;

    bool quiet_ = false;
    long long events_ = 0;
    std::vector<std::string> type_names_;
    std::vector<long long> type_counts_;
    RingBuffer<WindowEntry, kWindowCapacity> window_;
    RuleEngine engine_;
    long long skipped_ = 0;
    long long broken_ = 0;
};

}
#endif  // NANO_EDR_AGENT_H