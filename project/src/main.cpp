#include <charconv>
#include <cstring>
#include <fstream>
#include <print>
#include <string>
#include <vector>

#include "event_list.h"
#include "parse.h"

void Context (const nano_edr::EventList* list){
    nano_edr::EventNode* cntx1 = nullptr;
    nano_edr::EventNode* cntx2 = nullptr;
    nano_edr::EventNode* head = list->head;
    if (list -> size == 0){
        return;
    }
    if (list -> size == 1){
        nano_edr::Event* head1 = &head->event;
        std::print("[CTX] -1: ts={} type={} pid={}\n",head1->ts, head1->type, head1->pid);
        return;
    }
    cntx1 = head -> next;
    cntx2 = head;
    while (cntx1->next){
        cntx2 = cntx1;
        cntx1 = cntx1 -> next;
    }
    std::print("[CTX] -2: ts={} type={} pid={}\n",cntx2->event.ts, cntx2->event.type, cntx2->event.pid);
    std::print("[CTX] -1: ts={} type={} pid={}\n",cntx1->event.ts, cntx1->event.type, cntx1->event.pid);
    return;
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

int main(int argc, char** argv) {
    nano_edr::EventList list;

    if (argc < 2) {
        std::print(stderr, "использование: nano-edr <журнал.log> [--quiet] [--window-size N]\n");
        return 2;
    }

    bool quiet = false;
    size_t window_size = 64;
    std::string path;

    for (int i = 1; i < argc; i++){
        std::string arg = std::string(argv [i]);
        if (arg == "--quiet"){
            quiet = true;
            continue;
        }
        else if(arg == "--window-size"){
            if (i + 1 == argc){
                std::print(stderr, "использование: nano-edr [--quiet] [--window-size N]\n <N>\n");
                return 2;
            }
            i++;
            std::string s = argv[i];
            size_t n = 0;
            auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), n);
            if (ec != std::errc{} || ptr != s.data() + s.size()) {
                std::print(stderr, "неверное значение: {}\n", s);
                return 2;
            }
            window_size = n;
        }
        else if (path.empty()) {
            path = arg;
        }

    }
    list.capacity = window_size;

    if (path.empty()) {
        std::print(stderr, "использование: nano-edr <журнал.log> [--quiet] [--window-size N]\n");
        return 2;
    }

    long long lines = 0;
    long long events = 0;
    long long comments = 0;
    std::vector<std::string> type_names;
    std::vector<long long>   type_counts;
    std::string line;

    const std::vector<std::string> signs = {
        "wscript.exe",
        ".locked",
        "certutil.exe",
        "\\Startup\\",
    };

    std::ifstream log(path);

    if (!log) {
        std::print(stderr, "не удалось открыть журнал: {}\n", path);
        return 2;
    }

    nano_edr::Event cntx2;   // предпоследнее
    nano_edr::Event cntx1;   // последнее
    bool cntx2_valid = false;
    bool cntx1_valid = false;

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

        bool detect = false;
        for (const std::string& sign : signs) {
            if (line.find(sign) != std::string::npos) {
                std::print("[DETECT] строка {}, признак {}: {}\n",lines, sign, line);
                detect = true;
            }
        }
        if(detect && !quiet){
            std::size_t max_cntx = (list.capacity == 1) ? 1 : 2;
            if (max_cntx >= 2 && cntx2_valid) {
                std::print("[CTX] -2: ts={} type={} pid={}\n", cntx2.ts, cntx2.type, cntx2.pid);
            }
            if (max_cntx >= 1 && cntx1_valid) {
                std::print("[CTX] -1: ts={} type={} pid={}\n", cntx1.ts, cntx1.type, cntx1.pid);
            }
        }

        cntx2 = cntx1;
        cntx2_valid = cntx1_valid;
        cntx1 = event;
        cntx1_valid = true;

        nano_edr::ListPushBack(&list, &event);
    }

    if (!quiet) {
        std::print("строк {}, из них комментариев {}, событий {}\n", lines, comments, events);
        for (std::size_t j = 0; j < type_names.size(); ++j) {
            std::print("  {}: {}\n", type_names[j], type_counts[j]);
        }
    }
    return 0;
}
