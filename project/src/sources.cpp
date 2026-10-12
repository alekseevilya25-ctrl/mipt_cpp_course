#include "sources.h"

#include <exception>
#include <fstream>
#include <print>
#include <stdexcept>
#include <string>

#include "event.h"
#include "event_list.h"
#include "parse.h"

FileSource::FileSource(const std::string& path_): path(path_){}

void FileSource::Run(Agent* agent){
    std::ifstream log(path);
    if (!log) {
        throw std::runtime_error("не удалось открыть журнал: " + path);
    }
    std::string line;
    while (std::getline(log, line)){
        (agent)->IncLines();
        if (nano_edr::IsBlankOrComment(line)){
            (agent)->IncComments();
            continue;
        }
        nano_edr::EventParts parts;
        if (!nano_edr::ParseEventParts(line, &parts)){
            continue;
        }
        try {
            agent->HandleEvent(nano_edr::Event(parts));
        }
        catch (const std::exception& e){
            std::print(stderr, "{}\n", e.what());
        }
    }
}

OsSource::OsSource(const std::string& config_path): h_(config_path){}

void OsSource::Run(Agent* agent){
    h_.Subscribe(Agent::Trampoline, agent);
    h_.Start();
    os_status sts;
    while ((sts = h_.Wait(1000)) == OS_TIMEOUT){
    }
    if (sts != OS_OK){
        throw std::runtime_error("Ошибка os_source.run" + std::string(os_status_str(sts)));
    }
}