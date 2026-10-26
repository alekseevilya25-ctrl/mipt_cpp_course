#include "sources.h"

#include <exception>
#include <fstream>
#include <print>
#include <stdexcept>
#include <string>

#include "event.h"
#include "event_list.h"
#include "parse.h"

namespace nano_edr{

FileSource::FileSource(const std::string& path): log_(path){
    if (!log_) {
        throw std::runtime_error("не удалось открыть журнал: " + path);
    }
}

void FileSource::Run(IEventHandler* handler){
    std::string line;
    while (std::getline(log_, line)){
        try {
        if (IsBlankOrComment(line)){
            handler->OnSkipped();
            continue;
        }
        EventParts parts;
        if (!ParseEventParts(line, &parts)){
            handler->OnBroken();
            continue;
        }

        handler->HandleEvent(Event(parts));
        }
        catch (const std::exception& e){
            handler->OnBroken();
            std::print(stderr, "{}\n", e.what());
        }
    }
}

OsSource::OsSource(const std::string& config_path): h_(config_path){}

void OsSource::Run(IEventHandler* handler){
    h_.Subscribe(&OsSource::Trampoline, handler);
    h_.Start();
    os_status sts;
    while ((sts = h_.Wait(1000)) == OS_TIMEOUT){
        handler->OnTick();
    }
    if (sts != OS_OK){
        throw std::runtime_error("Ошибка os_source.run" + std::string(os_status_str(sts)));
    }
}

void OsSource::Trampoline(const os_event* ev, void* ctx) noexcept{
    IEventHandler* handler = static_cast<IEventHandler*>(ctx);
    try {
        EventParts parts;
        for (size_t i = 0; i < ev->field_count; i++){
            Field k;
            k.key = ev->fields[i].key;
            k.value = ev->fields[i].value;
            parts.fields.push_back(k);
        }
        parts.ts = std::to_string(ev->ts);
        parts.type = ev->type;
        parts.pid = ev->pid == 0 ? "" : std::to_string(ev->pid);
        handler->HandleEvent(Event(parts));
    }
    catch (const std::exception& e) {
        handler->OnBroken();
        std::print(stderr, "Trampoline: {}\n", e.what());
    }
    catch (...) {
        handler->OnBroken();
        std::print(stderr, "Trampoline: неизвестная ошибка\n");
    }
}

}