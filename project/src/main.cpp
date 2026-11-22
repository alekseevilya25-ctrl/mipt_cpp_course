#include <charconv>
#include <cstring>
#include <print>
#include <string>

#include "agent.h"
#include "sources.h"
#include "alert_sink.h"
#include "edr_handle.h"
#include "field_traits.h"
#include "copy_stats.h"

void Error(){
    std::print(stderr, "использование: nano-edr <журнал.log> [--quiet] [--copies] [--dry-run]\n");
}

bool ParseArgs(int argc, char** argv, std::string& path, bool& quiet, bool& use_file, std::string& out, bool& copies, bool& dry_run){
    int pathcnt = 0;
    for (int i = 1; i < argc; i++){
        std::string arg = std::string(argv[i]);
        if (arg == "--quiet"){
            quiet = true;
        }
        else if (arg == "--out"){
            if (i + 1 == argc){
                Error();
                return false;
            }
            out = argv[++i];
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
        else if (arg == "--copies"){
            copies = true;
        }
        else if (arg == "--dry-run"){
            dry_run = true;
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
        std::string out;
        std::string path;
        bool use_file = false;
        bool copies = false;
        bool dry_run = false;

        if (!ParseArgs(argc, argv, path, quiet, use_file, out, copies, dry_run)){
            return 2;
        }

        nano_edr::EdrHandle edr(path);
        const std::string text = edr.Config("dedup_period_ms", "10000");
        uint64_t dedup_period_ms = 0;
        if (!nano_edr::FieldTraits<uint64_t>::Parse(text, &dedup_period_ms)) {
            std::print(stderr, "[WARN] dedup_period_ms не разбирается: {}\n", text);
            dedup_period_ms = 10000;
        }
        nano_edr::Agent agent(quiet, dedup_period_ms);
        nano_edr::FanOutSink targets;

        targets.Add(nano_edr::ConsoleSink());
        targets.Add(nano_edr::EdrSink(&edr));

        if (!out.empty()) {
            targets.Add(nano_edr::FileSink(out));
        }

        agent.Subscribe(std::move(targets));

        std::unique_ptr<nano_edr::IEventSource> source;
        if (use_file) {
            source = std::make_unique<nano_edr::FileSource>(path);
        } 
        else {
            source = std::make_unique<nano_edr::OsSource>(path);
        }

        agent.EnableResponse(source->os(), dry_run, edr.Config("quarantine_dir", "C:\\quarantine"));
        
        source->Run(&agent);
        if (edr.Flush() != EDR_OK) {
            std::print(stderr, "[WARN] детекты не отправлены\n");
        }
        agent.PrintSummary();
        
        if (copies){
            const auto& s = nano_edr::copy_stats();
            std::print("[COPY] строк события скопировано {}, перемещено {}\n", s.event_strings_copied, s.event_strings_moved);
            std::print("[COPY] записей WindowEntry скопировано {}, перемещено {}\n", s.window_copied, s.window_moved);
            std::print("[COPY] событий скопировано в детекты {}\n", s.evidence_copied);
            std::print("[COPY] копий всего {}\n", s.total());
        }
    }
    catch (const std::exception& error){
        std::print(stderr, "ошибка: {}\n", error.what());
        return 1;
    }
}