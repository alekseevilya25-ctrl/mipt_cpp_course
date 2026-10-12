#include "os_handle.h"
#include <stdexcept>
#include <string>
namespace nano_edr{

OsHandle::OsHandle(const std::string& config_path){
    os_status sts = os_init(config_path.c_str(), &handle_);
    if (sts != OS_OK){
        throw std::runtime_error("не удалось инициализировать: " + config_path + "->" + std::string(os_status_str(sts)));
    }
}

OsHandle::~OsHandle(){
    if(handle_){
        os_free(handle_);
    }
}

void OsHandle::Subscribe(os_event_cb callback, void* context){
    os_status sts = os_event_subscribe(handle_, callback, context);
    if(sts != OS_OK){
        throw std::runtime_error("не удалось подписаться: " + std::string(os_status_str(sts)));
    }
}
void OsHandle::Start (){
    os_status sts = os_start (handle_);
    if(sts != OS_OK){
        throw std::runtime_error("не удалось запустить: " + std::string(os_status_str(sts)));
    }
}
os_status OsHandle::Wait (uint32_t timeout_ms){
    return os_wait (handle_, timeout_ms);
}
void OsHandle::Stop(){
    os_stop(handle_);
}
}