#include "detection.h"
#include "rules.h"
#include <string>

namespace nano_edr {

std::string ToString(const Detection& detection){
    std::string str;
    str += "[DETECT] ";
    str += SeverityName(detection.severity);
    str += "  ";
    str += detection.rule;
    str += "  ts=";
    str += std::to_string(detection.ts.ms);
    str += " pid=";
    str += detection.pid;
    if (!detection.chain.empty()){
        str += " chain=";
        str += detection.chain;
    }
    return str;
}

}