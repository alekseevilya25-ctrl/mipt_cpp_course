#include "event_list.h"

namespace nano_edr {

EventList::~EventList() {
    ListClear(this);
}

void ListPushBack(EventList* list, const Event* event){
    EventNode* new_node = new EventNode;
    new_node -> event = *event;
    if (!(list -> head)){
        list -> head = new_node;
        list -> tail = list -> head;
        list -> size = 1;
        return;
    }
    list->prev_tail = list->tail;
    list -> tail -> next = new_node;
    list -> tail = new_node;
    ++list -> size;
    if (list->capacity > 0 && list->size > list->capacity){
        ListPopFront(list);
    }
}
void ListPopFront(EventList* list){
    if (list -> head == nullptr){
        return;
    }
    EventNode* oldhead = list -> head;
    list -> head = list -> head -> next;
    if (list -> head == nullptr){
        list -> tail = nullptr;
        list -> prev_tail = nullptr;
    }
    else if (list -> size == 2){
        list -> prev_tail = nullptr;
    }
    delete oldhead;
    list -> size--;
}
void ListClear(EventList* list){
    while (list -> size >=1){
        ListPopFront(list);
    }
}
}