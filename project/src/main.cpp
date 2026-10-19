#include <charconv>
#include <cstring>
#include <print>
#include <string>

#include "agent.h"
#include "sources.h"

void Error(){
    std::print(stderr, "использование: nano-edr <журнал.log> [--quiet] [--window-size N]\n");
}

bool ParseArgs(int argc, char** argv, std::string& path, bool& quiet, std::size_t& window_size, bool& use_file){
    int pathcnt = 0;
    for (int i = 1; i < argc; i++){
        std::string arg = std::string(argv[i]);
        if (arg == "--quiet"){
            quiet = true;
        }
        else if (arg == "--window-size"){
            if (i + 1 == argc) {
                Error();
                return false;
            }
            i++;
            std::string s = argv[i];
            size_t n = 0;
            auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), n);
            if (ec != std::errc{} || ptr != s.data() + s.size()){
                std::print(stderr, "неверное значение: {}\n", s);
                return false;
            }
            window_size = n;
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
        size_t window_size = 64;
        std::string path;
        bool use_file = false;
        if (!ParseArgs(argc, argv, path, quiet, window_size, use_file)){
            return 2;
        }
        nano_edr::Agent agent(window_size, quiet);
        if (use_file){
            nano_edr::FileSource(path).Run(&agent);
        }
        else{
            nano_edr::OsSource(path).Run(&agent);
        }
        agent.PrintSummary();
    }
    catch (const std::exception& error){
        std::print(stderr, "ошибка: {}\n", error.what());
        return 1;
    }
}