#include "agent_rules.h"
#include <memory>
#include "rule_engine.h"
#include "make_rule.h"
#include "combinators.h"
namespace nano_edr{

RuleEngine BuildAgentRules(const EntityModel* model){
    RuleEngine engine;
    engine.SetModel(model);
    auto rule1 = std::make_unique<MatchRule>("ransom_extension", Severity::kCritical);
    rule1->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kFileCreate, EventType::kFileWrite, EventType::kFileMove}));
    
    auto paths1 = std::make_unique<AnyOf>();
    paths1->Add(std::make_unique<FieldEndsWith>("path", ".locked"));
    paths1->Add(std::make_unique<FieldEndsWith>("to", ".locked"));
    rule1->AddCondition(std::move(paths1));

    rule1->AddAction("quarantine_file");
    engine.AddRule(std::move(rule1));

    auto rule2 = std::make_unique<MatchRule>("script_host_from_temp", Severity::kHigh);
    rule2->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
    
    auto image2 = std::make_unique<AnyOf>();
    image2->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
    image2->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
    rule2->AddCondition(std::move(image2));
    
    auto cmd2 = std::make_unique<AnyOf>();
    cmd2->Add(std::make_unique<FieldContains>("cmdline", "\\appdata\\local\\temp\\"));
    cmd2->Add(std::make_unique<FieldContains>("cmdline", "/appdata/local/temp/"));
    cmd2->Add(std::make_unique<FieldContains>("cmdline", "\\windows\\temp\\"));
    cmd2->Add(std::make_unique<FieldContains>("cmdline", "/windows/temp/"));
    cmd2->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
    cmd2->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
    rule2->AddCondition(std::move(cmd2));
    
    rule2->AddAction("kill_process");
    engine.AddRule(std::move(rule2));

    auto rule3 = std::make_unique<MatchRule>("lolbin_download", Severity::kHigh);
    rule3->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
    
    auto image3 = std::make_unique<AnyOf>();
    image3->Add(std::make_unique<FieldEndsWith>("image", "certutil.exe"));
    image3->Add(std::make_unique<FieldEndsWith>("image", "bitsadmin.exe"));
    rule3->AddCondition(std::move(image3));
    
    auto cmd3 = std::make_unique<AnyOf>();
    cmd3->Add(std::make_unique<FieldContains>("cmdline", "transfer"));
    cmd3->Add(std::make_unique<FieldContains>("cmdline", "http:"));
    cmd3->Add(std::make_unique<FieldContains>("cmdline", "urlcache"));
    cmd3->Add(std::make_unique<FieldContains>("cmdline", "https:"));
    rule3->AddCondition(std::move(cmd3));
    
    rule3->AddAction("kill_process");
    engine.AddRule(std::move(rule3));

    auto rule4 = std::make_unique<MatchRule>("hidden_powershell", Severity::kMedium);
    rule4->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
    
    auto image4 = std::make_unique<AnyOf>();
    image4->Add(std::make_unique<FieldEndsWith>("image", "powershell.exe"));
    image4->Add(std::make_unique<FieldEndsWith>("image", "pwsh.exe"));
    rule4->AddCondition(std::move(image4));
    
    auto cmd4 = std::make_unique<AnyOf>();
    cmd4->Add(std::make_unique<FieldContains>("cmdline", "-w hidden"));
    cmd4->Add(std::make_unique<FieldContains>("cmdline", "-windowstyle hidden"));
    cmd4->Add(std::make_unique<FieldContains>("cmdline", "-enc"));
    cmd4->Add(std::make_unique<FieldContains>("cmdline", "-encodedcommand"));
    rule4->AddCondition(std::move(cmd4));
    
    rule4->AddAction("kill_process");
    engine.AddRule(std::move(rule4));

    auto rule5 = std::make_unique<MatchRule>("autostart_write", Severity::kHigh);
    rule5->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kFileCreate, EventType::kFileWrite, EventType::kFileMove}));
    
    auto paths5 = std::make_unique<AnyOf>();
    paths5->Add(std::make_unique<FieldContains>("path", "\\start menu\\programs\\startup\\"));
    paths5->Add(std::make_unique<FieldContains>("path", "/start menu/programs/startup/"));
    paths5->Add(std::make_unique<FieldContains>("to", "\\start menu\\programs\\startup\\"));
    paths5->Add(std::make_unique<FieldContains>("to", "/start menu/programs/startup/"));
    rule5->AddCondition(std::move(paths5));
    
    rule5->AddAction("quarantine_file");
    engine.AddRule(std::move(rule5));

    auto rule6 = std::make_unique<SequenceRule>("script_host_beacon", Severity::kCritical, 10000);

    auto first = std::make_unique<AllOf>();
    first->Add(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
    auto image = std::make_unique<AnyOf>();
    image->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
    image->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
    first->Add(std::move(image));

    auto cmd = std::make_unique<AnyOf>();
    cmd->Add(std::make_unique<FieldContains>("cmdline", "\\appdata\\local\\temp\\"));
    cmd->Add(std::make_unique<FieldContains>("cmdline", "/appdata/local/temp/"));
    cmd->Add(std::make_unique<FieldContains>("cmdline", "\\windows\\temp\\"));
    cmd->Add(std::make_unique<FieldContains>("cmdline", "/windows/temp/"));
    cmd->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
    cmd->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
    first->Add(std::move(cmd));

    rule6->SetFirst(std::move(first));

    rule6->SetSecond(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kNetConnect}));
    rule6->AddAction("kill_process");
    engine.AddRule(std::move(rule6));

    auto mass_write = MakeRule<ThresholdRule<128>>("ransomware_mass_write", Severity::kCritical, 20, 10000, "pid");

    mass_write->SetCondition(MakeCondition(AllOfStatic(EventTypeIs({EventType::kFileCreate, EventType::kFileWrite, EventType::kFileMove}), AnyOfStatic(FieldContains("path", "\\Documents\\"), FieldContains("path", "\\Desktop\\"), FieldContains("to",   "\\Documents\\"), FieldContains("to",   "\\Desktop\\")))));

    mass_write->AddAction("kill_process");
    mass_write->AddAction("quarantine_file");
    engine.AddRule(std::move(mass_write)); 
    
    auto shell = MakeRule<SequenceRule>("shell_spawns_script_host", Severity::kHigh, 30000);

    auto shell_first = std::make_unique<AllOf>();

    shell_first->Add(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));

    auto shell_first_image = std::make_unique<AnyOf>();
    shell_first_image->Add(std::make_unique<FieldEndsWith>("image", "cmd.exe"));
    shell_first_image->Add(std::make_unique<FieldEndsWith>("image", "powershell.exe"));
    shell_first_image->Add(std::make_unique<FieldEndsWith>("image", "pwsh.exe"));
    shell_first->Add(std::move(shell_first_image));

    shell->SetFirst(std::move(shell_first));


    auto shell_second = std::make_unique<AllOf>();

    shell_second->Add(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));

    auto shell_second_image = std::make_unique<AnyOf>();
    shell_second_image->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
    shell_second_image->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
    shell_second->Add(std::move(shell_second_image));

    shell->SetSecond(std::move(shell_second));

    shell->set_child_of_prev(true);
    shell->SetModel(model);
    shell->AddAction("kill_process");
    engine.AddRule(std::move(shell));

    auto dropped = MakeRule<LambdaRule>(
        "spawned_from_dropped_file", Severity::kHigh,
        [model](const Event& event) {
            if (event.event_type() != EventType::kProcessStart) {
                return false;
            }
            const FileRecord* source = model->FindSpawnSource(event.pid());
            if (source == nullptr) {
                return false;
            }
            if (source->writer_pid.empty() || source->writer_pid == event.pid()){
                return false;
            }
            return true;
        });
    dropped->AddAction("kill_process");
    dropped->AddAction("quarantine_file");
    engine.AddRule(std::move(dropped));

    return engine;
}

}