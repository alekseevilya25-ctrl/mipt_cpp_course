#ifndef NANO_EDR_SOURCES_H
#define NANO_EDR_SOURCES_H

#include <string>
#include <fstream>

#include "agent.h"
#include "os_handle.h"

namespace nano_edr{

class FileSource{
public:
    explicit FileSource(const std::string& path_);
    void Run(Agent* agent);

private:
    std::ifstream log_;
};

class OsSource{
public:
    explicit OsSource(const std::string& config_path);
    void Run(Agent* agent);

private:
    OsHandle h_;
};

}
#endif