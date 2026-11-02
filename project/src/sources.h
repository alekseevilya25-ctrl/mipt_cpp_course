#ifndef NANO_EDR_SOURCES_H
#define NANO_EDR_SOURCES_H

#include <string>
#include <fstream>

#include "os_handle.h"
#include "event_source.h"

namespace nano_edr{

class FileSource : public IEventSource{
public:
    explicit FileSource(const std::string& path_);
    void Run(IEventHandler* agent) override;

private:
    std::ifstream log_;
};

class OsSource : public IEventSource{
public:
    explicit OsSource(const std::string& config_path);
    void Run(IEventHandler* agent) override;
    static void Trampoline(const os_event* ev, void* ctx) noexcept;

private:
    OsHandle h_;
};

}
#endif