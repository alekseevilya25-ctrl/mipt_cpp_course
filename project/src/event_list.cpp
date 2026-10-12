#include "event_list.h"

namespace nano_edr {

EventList::~EventList() {
    Clear();
}
void EventList::Clear(){
    while (size_ >= 1){
        PopFront();
    }
}
void EventList::PushBack(const Event& event){
    if (!(head_)){
        head_ = new EventNode(event);
        tail_ = head_;
        size_ = 1;
        return;
    }
    EventNode* new_node = new EventNode(event);
    tail_ -> next = new_node;
    tail_ = new_node;
    ++size_;
    if (capacity_ > 0 && size_ > capacity_){
        PopFront();
    }
}
void EventList::PopFront(){
    if (size_ == 1){
        delete head_;
        size_ --;
        head_ = nullptr;
        tail_ = nullptr;
        return;
    }
    if (head_ == nullptr){
        return;
    }
    EventNode* oldhead = head_;
    head_ = head_ -> next;
    delete oldhead;
    size_ --;
}
}