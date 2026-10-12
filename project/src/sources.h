#ifndef NANO_EDR_SOURCES_H
#define NANO_EDR_SOURCES_H

#include <string>

#include "agent.h"
#include "os_handle.h"

class FileSource{
public:
    explicit FileSource(const std::string& path_);
    void Run(Agent* agent);

private:
    std::string path;
};

class OsSource{
public:
    explicit OsSource(const std::string& config_path);
    void Run(Agent* agent);

private:
    nano_edr::OsHandle h_;
};

#endif