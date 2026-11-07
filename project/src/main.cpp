#include <charconv>
#include <cstring>
#include <print>
#include <string>

#include "agent.h"
#include "sources.h"

void Error(){
    std::print(stderr, "использование: nano-edr <журнал.log> [--quiet]\n");
}

bool ParseArgs(int argc, char** argv, std::string& path, bool& quiet, bool& use_file){
    int pathcnt = 0;
    for (int i = 1; i < argc; i++){
        std::string arg = std::string(argv[i]);
        if (arg == "--quiet"){
            quiet = true;
        }
        else if (arg == "--file"){
            use_file = true;
            if (i + 1 == argc){
                Error();
                return false;
            }
            path = argv[++i];
            pathcnt++;
        }
        else if (arg[0] == '-'){
            Error();
            return false;
        }
        else{
            path = arg;
            ++pathcnt;
        }
    }

    if (path.empty() || pathcnt > 1){
        Error();
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    try{
        bool quiet = false;
        std::string path;
        bool use_file = false;
        if (!ParseArgs(argc, argv, path, quiet, use_file)){
            return 2;
        }
        nano_edr::Agent agent(quiet);
        std::unique_ptr<nano_edr::IEventSource> source;
        if (use_file) {
            source = std::make_unique<nano_edr::FileSource>(path);
        } 
        else {
            source = std::make_unique<nano_edr::OsSource>(path);
        }
        source->Run(&agent);
        agent.PrintSummary();
    }
    catch (const std::exception& error){
        std::print(stderr, "ошибка: {}\n", error.what());
        return 1;
    }
}