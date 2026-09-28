#include "agent_rules.h"
#include "fields.h"
namespace nano_edr{

bool StrContains(const std::string& s, const std::string& needle){
    const std::string* cmd = &s;
    std::string cmd_ = NormalizePath(*cmd);
    std::string needle_ = NormalizePath(needle);
    if (cmd_.find(needle_) != std::string::npos){
        return true;
    }
    else{
        return false;
    }
}
bool IsFileCreate(const Event& event){
    if (event.type == "file_create") return true;
    return false;
}
bool IsFileMove(const Event& event){
    if (event.type == "file_move") return true;
    return false;
}
bool StrEndsWith(const std::string& s, const std::string& suffix){
    const std::string* path = &s;
    std::string npath = NormalizePath(*path);
    std::string nsuffix = NormalizePath(suffix);
    if (nsuffix.size() > npath.size()){
        return false;
    }
    size_t pos = npath.size() - nsuffix.size();
    if (npath.find(nsuffix, pos) == pos){
       return true;
    }
    return false;
}


bool ScriptHostFromTemp(const Event& event){
    if (!IsProcessStart(event)){
        return false;
    }
    const std::string& s = GetRequiredField(event, "image");
    if (StrEndsWith(s, "wscript.exe") || StrEndsWith(s, "cscript.exe")){
        if (CommandLineContains(event, "\\appdata\\local\\temp\\") || CommandLineContains(event, "\\windows\\temp\\")) {
            return true;
        }
        return false;
    }
    return false;
}
bool LolbinDownload(const Event& event){
    if (!IsProcessStart(event)){
        return false;
    }
    const std::string& s = GetRequiredField(event, "image");
    if (StrEndsWith(s, "certutil.exe") || StrEndsWith(s, "bitsadmin.exe")){
        if (CommandLineContains(event, "transfer") || CommandLineContains(event, "http:") || CommandLineContains(event, "urlcache") || CommandLineContains(event, "https:")) {
            return true;
        }
        return false;
    }
    return false;
}
bool HiddenPowershell(const Event& event){
    if (!IsProcessStart(event)){
        return false;
    }
    const std::string& s = GetRequiredField(event, "image");
    if (StrEndsWith(s, "powershell.exe") || StrEndsWith(s, "pwsh.exe")){
        if (CommandLineContains(event, "-w hidden") || CommandLineContains(event, "-windowstyle hidden") || CommandLineContains(event, "-enc") || CommandLineContains(event, "-encodedcommand")) {
            return true;
        }
        return false;
    }
    return false;
}
bool AutostartWrite(const Event& event){
    if(IsFileWrite(event) || IsFileCreate(event)){
        const std::string& s = GetRequiredField(event, "path");
        if(StrContains(s, "\\start menu\\programs\\startup\\")){
            return true;
        }
        return false;
    }
    if(IsFileMove(event)){
        const std::string& s = GetRequiredField(event, "to");
        if(StrContains(s, "\\start menu\\programs\\startup\\")){
            return true;
        }
        return false;
    }
    return false;
}
bool RansomExtension(const Event& event){
    if(IsFileWrite(event) || IsFileCreate(event)){
        const std::string& s = GetRequiredField(event, "path");
        if(StrEndsWith(s, ".locked")){
            return true;
        }
        return false;
    }
    if(IsFileMove(event)){
        const std::string& s = GetRequiredField(event, "to");
        if(StrEndsWith(s, ".locked")){
            return true;
        }
        return false;
    }
    return false;
}
constexpr Rule kRules[] = {
    {"script_host_from_temp", ScriptHostFromTemp, Severity::kHigh},
    {"lolbin_download",       LolbinDownload,     Severity::kHigh},
    {"hidden_powershell",     HiddenPowershell,   Severity::kMedium},
    {"autostart_write",       AutostartWrite,     Severity::kHigh},
    {"ransom_extension",      RansomExtension,    Severity::kCritical},
};
const Rule* AgentRules(){
    return kRules;
}
size_t AgentRuleCount(){
    return sizeof(kRules) / sizeof(kRules[0]);
}
}