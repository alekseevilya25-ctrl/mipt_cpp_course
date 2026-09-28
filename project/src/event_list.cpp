#include "event_list.h"

namespace nano_edr {

EventList::~EventList() {
    ListClear(this);
}

void ListPushBack(EventList* list, const Event* event){
    if (!(list -> head)){
        list -> head = new EventNode;
        list -> head -> event = *event;
        list -> head -> next = nullptr;
        list -> tail = list -> head;
        list -> size = 1;
        return;
    }
    EventNode* new_node = new EventNode;
    new_node -> event = *event;
    list -> tail -> next = new_node;
    list -> tail = new_node;
    ++list -> size;
    if (list->capacity > 0 && list->size > list->capacity){
        ListPopFront(list);
    }
}
void ListPopFront(EventList* list){
    if (list -> size == 1){
        delete list -> head;
        list -> size --;
        list -> head = nullptr;
        list -> tail = nullptr;
        return;
    }
    if (list -> head == nullptr){
        return;
    }
    EventNode* oldhead = list -> head;
    list -> head = list -> head -> next;
    delete oldhead;
    list -> size--;
}
void ListClear(EventList* list){
    while (list -> size >=1){
        ListPopFront(list);
    }
}
}