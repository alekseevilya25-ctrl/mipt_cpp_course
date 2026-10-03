#include <charconv>
#include <cstring>
#include <fstream>
#include <print>
#include <string>
#include <vector>

#include "event_list.h"
#include "parse.h"
#include "fields.h"
#include "rules.h"
#include "agent_rules.h"


void Context(nano_edr::EventNode* ctx){
    if(!ctx){
        return;
    }
    int i = 1;
    if(ctx -> next){
        i++;
    }
    while(ctx){
        std::print("[CTX] -{}: ts={}  type={} pid={}\n", i, ctx->event.ts, ctx->event.type, ctx->event.pid);
        i--;
        ctx = ctx->next;
    }
}

void Typecnt (const std::string& type, std::vector<std::string>& names, std::vector<long long>& counts){
    for (std::size_t i = 0; i < names.size(); ++i) {
        if (names[i] == type) {
            ++counts[i];
            return;
        }
    }
    names.push_back(type);
    counts.push_back(1);
}

void Error(){
    std::print(stderr, "использование: nano-edr <журнал.log> [--quiet] [--window-size N]\n");
}

bool ParseArgs(int argc, char** argv, std::string& path, bool& quiet, std::size_t& window_size){
    for (int i = 1; i < argc; i++){
        std::string arg = std::string(argv [i]);
        if (arg == "--quiet"){
            quiet = true;
        }
        else if(arg == "--window-size"){
            if (i + 1 == argc){
                Error();
                return false;
            }
            i++;
            std::string s = argv[i];
            size_t n = 0;
            auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), n);
            if (ec != std::errc{} || ptr != s.data() + s.size()) {
                std::print(stderr, "неверное значение: {}\n", s);
                return false;
            }
            window_size = n;
        }
        else if (path.empty()) {
            path = arg;
        }

    }

    if (path.empty()) {
        Error();
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    try{
    nano_edr::EventList list;

    bool quiet = false;
    size_t window_size = 64;
    std::string path;
    if (!ParseArgs(argc, argv, path, quiet, window_size)){
        return 2;
    }
    list.capacity = window_size;

    long long lines = 0;
    long long events = 0;
    long long comments = 0;
    std::vector<std::string> type_names;
    std::vector<long long>   type_counts;
    std::string line;

   
    std::ifstream log(path);

    if (!log) {
        std::print(stderr, "не удалось открыть журнал: {}\n", path);
        return 2;
    }
    nano_edr::EventNode* ctx = nullptr;

    while (std::getline(log, line)) {
        nano_edr::Event event;
        ++lines;
        if (nano_edr::IsBlankOrComment(&line)){
            comments++;
            continue;
        }
        if (!nano_edr::ParseEventLine(&line, &event)){
            continue;
        }
        events++;
        
        Typecnt(event.type, type_names, type_counts);
        int n = nano_edr::CheckRules(event, nano_edr::AgentRules(), nano_edr::AgentRuleCount());
        if (n >= 1){
            Context(ctx);
        }
        nano_edr::ListPushBack(&list, &event);
        if(list.size <= 2){
            ctx = list.head;
        }
        else{
            ctx = ctx->next;
        }
    }

    if (!quiet) {
        std::print("строк {}, из них комментариев {}, событий {}\n", lines, comments, events);
        for (std::size_t j = 0; j < type_names.size(); ++j) {
            std::print("  {}: {}\n", type_names[j], type_counts[j]);
        }
    }
    return 0;
    }
    catch (const std::exception& error){
        std::print(stderr, "ошибка: {}\n", error.what());
        return 1;
    }
}
