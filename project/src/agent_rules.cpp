#include "agent_rules.h"

namespace nano_edr{

std::vector<MatchRule> BuildAgentRules(){
    std::vector<MatchRule> rules;
    MatchRule ransom_by_path("ransom_extension", Severity::kCritical);
    ransom_by_path.AddCondition(Condition::Equals("type", {"file_create", "file_write"}));
    ransom_by_path.AddCondition(Condition::EndsWith("path", {".locked"}));
    ransom_by_path.AddAction("quarantine_file");
    rules.push_back(ransom_by_path);

    MatchRule ransom_by_to("ransom_extension", Severity::kCritical);
    ransom_by_to.AddCondition(Condition::Equals("type", {"file_move"}));
    ransom_by_to.AddCondition(Condition::EndsWith("to", {".locked"}));
    ransom_by_to.AddAction("quarantine_file");
    rules.push_back(ransom_by_to);

    MatchRule script_host_from_temp("script_host_from_temp", Severity::kHigh);
    script_host_from_temp.AddCondition(Condition::Equals("type", {"process_start"}));
    script_host_from_temp.AddCondition(Condition::EndsWith("image", {"wscript.exe", "cscript.exe"}));
    script_host_from_temp.AddCondition(Condition::Contains("cmdline", {
        "\\appdata\\local\\temp\\", "/appdata/local/temp/",
        "\\windows\\temp\\", "/windows/temp/",
        "%temp%", "%tmp%"}));
    script_host_from_temp.AddAction("kill_process");
    rules.push_back(script_host_from_temp);

    MatchRule lolbin_download("lolbin_download", Severity::kHigh);
    lolbin_download.AddCondition(Condition::Equals("type", {"process_start"}));
    lolbin_download.AddCondition(Condition::EndsWith("image", {"certutil.exe", "bitsadmin.exe"}));
    lolbin_download.AddCondition(Condition::Contains("cmdline", {"transfer", "http:", "urlcache", "https:"}));
    lolbin_download.AddAction("kill_process");
    rules.push_back(lolbin_download);

    MatchRule hidden_powershell("hidden_powershell", Severity::kMedium);
    hidden_powershell.AddCondition(Condition::Equals("type", {"process_start"}));
    hidden_powershell.AddCondition(Condition::EndsWith("image", {"powershell.exe", "pwsh.exe"}));
    hidden_powershell.AddCondition(Condition::Contains("cmdline", {"-w hidden", "-windowstyle hidden", "-enc", "-encodedcommand"}));
    hidden_powershell.AddAction("kill_process");
    rules.push_back(hidden_powershell);

    MatchRule autostart_by_path("autostart_write", Severity::kHigh);
    autostart_by_path.AddCondition(Condition::Equals("type", {"file_create", "file_write"}));
    autostart_by_path.AddCondition(Condition::Contains("path", {"\\start menu\\programs\\startup\\", "/start menu/programs/startup/"}));
    autostart_by_path.AddAction("quarantine_file");
    rules.push_back(autostart_by_path);

    MatchRule autostart_by_to("autostart_write", Severity::kHigh);
    autostart_by_to.AddCondition(Condition::Equals("type", {"file_move"}));
    autostart_by_to.AddCondition(Condition::Contains("to", {"\\start menu\\programs\\startup\\", "/start menu/programs/startup/"}));
    autostart_by_to.AddAction("quarantine_file");
    rules.push_back(autostart_by_to);

    return rules;
}

}