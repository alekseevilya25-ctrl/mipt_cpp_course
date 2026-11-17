#include <string>
#include <print>

#include "detection.h"
#include "entity_model.h"
#include "event.h"
#include "os_handle.h"
#include "response.h"
#include "fields.h"

namespace nano_edr {

ResponsePlanner::ResponsePlanner(const EntityModel* model, std::string quarantine_dir): model_(model), quarantine_dir_(std::move(quarantine_dir)) {}

Responder::Responder(OsHandle* os, bool dry_run): os_(os), dry_run_(dry_run) {}

static bool IsSystemPath(const std::string& path){
    std::string lower = path;
    for (char& c : lower){
        if (c >= 'A' && c <= 'Z'){
            c += 32;
        }
    }
    return lower.rfind("c:\\windows\\", 0) == 0 || lower.rfind("c:\\program files", 0) == 0;
}

std::string ResponsePlanner::QuarantinePath(const std::string& path, const std::string& pid) const {
    if (quarantine_dir_.empty()) return "";
    const std::size_t i = path.find_last_of("\\");
    const std::string name = (i == std::string::npos) ? path : path.substr(i + 1);
    return quarantine_dir_ + "\\" + pid + "-" + name;
}

ResponsePlan ResponsePlanner::Plan(const Detection& detection) const {
    ResponsePlan plan;
    const std::vector<std::string> tree = model_->GetProcessTree(detection.pid);

    for (const std::string& name : detection.actions) {
        if (name == "kill_process") {
            for (std::size_t i = tree.size(); i > 0; --i) {
                const std::string& pid = tree[i - 1];
                const ProcessRecord* rec = model_->FindProcess(pid);
                Action action;
                action.kind = ActionKind::kKillProcess;
                action.pid = pid;
                action.expected_start = rec ? rec->start : Timestamp{};
                action.reason = detection.rule;
                if (pid != detection.pid) {
                    action.reason += ", потомок " + detection.pid;
                }
                plan.Add(std::move(action));
            }
        } 
        else if (name == "quarantine_source") {
            const FileRecord* source = model_->FindSpawnSource(detection.pid);
            if (source == nullptr) continue;
            if (IsSystemPath(source->path)) continue;
            const std::string to = QuarantinePath(source->path, detection.pid);
            if (to.empty()) continue;
            Action action;
            action.kind = ActionKind::kQuarantineFile;
            action.path = source->path;
            action.to = to;
            action.reason = detection.rule + ", источник запуска";
            plan.Add(std::move(action));
        } 
        else if (name == "quarantine_image") {
            const ProcessRecord* rec = model_->FindProcess(detection.pid);
            if (rec == nullptr || rec->image.empty()) continue;
            if (IsSystemPath(rec->image)) continue;
            const std::string to = QuarantinePath(rec->image, detection.pid);
            if (to.empty()) continue;
            Action action;
            action.kind = ActionKind::kQuarantineFile;
            action.path = rec->image;
            action.to = to;
            action.reason = detection.rule + ", образ процесса";
            plan.Add(std::move(action));
        } 
        else if (name == "delete_dropped") {
            for (const std::string& pid : tree) {
                for (const std::string& path : model_->GetFilesCreatedBy(pid, true)) {
                    if (IsSystemPath(path)) continue;
                    Action action;
                    action.kind = ActionKind::kDeleteFile;
                    action.path = path;
                    action.reason = detection.rule + ", принесён " + pid;
                    plan.Add(std::move(action));
                }
            }
        } 
        else if (name == "delete_evidence") {
            for (const auto& ev_ptr : detection.evidence) {
                const Event& ev = *ev_ptr;
                const std::string* path = nullptr;
                if (ev.event_type() == EventType::kFileMove) {
                    path = FindField(ev, "to");
                } 
                else {
                    path = FindField(ev, "path");
                }
                if (path == nullptr) continue;
                if (IsSystemPath(*path)) continue;
                Action action;
                action.kind = ActionKind::kDeleteFile;
                action.path = *path;
                action.reason = detection.rule + ", файл из детекта";
                plan.Add(std::move(action));
            }
        }
    }
    return plan;
}

void Responder::Execute(const ResponsePlan& plan) {
    for (const Action& action : plan.actions()) {
        ++stats_.planned;

        bool repeat = false;
        for (const std::string& target : done_) {
            if (target == action.target()) {
                repeat = true;
                break;
            }
        }
        if (repeat) {
            ++stats_.skipped_repeat;
            continue;
        }
        done_.push_back(action.target());

        if (dry_run_ || os_ == nullptr) {
            std::print("[PLAN] {}\n", ToString(action));
            continue;
        }
        switch (action.kind) {
            case ActionKind::kKillProcess:
                Kill(action);
                break;
            case ActionKind::kQuarantineFile:
                MoveToQuarantine(action);
                break;
            case ActionKind::kDeleteFile:
                Delete(action);
                break;
        }
    }
}

}  // namespace nano_edr
