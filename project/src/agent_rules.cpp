//test
#include "agent_rules.h"
#include "rule_engine.h"
#include <memory>

namespace nano_edr{

RuleEngine BuildAgentRules(){
    RuleEngine engine;
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

    return engine;
}

}