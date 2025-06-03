#include "event_queue.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "event.h"

EventQueue::EventQueue() {
  // Initialize empty heap
}

EventQueue::~EventQueue() {
  // Clean up any remaining events
  Clear();
}

bool EventQueue::CompareEvents(Event* a, Event* b) const {
  return a->time() < b->time();
}

void EventQueue::HeapifyUp(size_t index) {
  if (index == 0) return;
  
  size_t parent_index = (index - 1) / 2;
  if (CompareEvents(min_heap_[index], min_heap_[parent_index])) {
    std::swap(min_heap_[index], min_heap_[parent_index]);
    HeapifyUp(parent_index);
  }
}

void EventQueue::HeapifyDown(size_t index) {
  size_t left_child = 2 * index + 1;
  size_t right_child = 2 * index + 2;
  size_t smallest = index;
  
  if (left_child < min_heap_.size() && 
      CompareEvents(min_heap_[left_child], min_heap_[smallest])) {
    smallest = left_child;
  }
  
  if (right_child < min_heap_.size() && 
      CompareEvents(min_heap_[right_child], min_heap_[smallest])) {
    smallest = right_child;
  }
  
  if (smallest != index) {
    std::swap(min_heap_[index], min_heap_[smallest]);
    HeapifyDown(smallest);
  }
}

void EventQueue::AddEvent(Event* event) {
  min_heap_.push_back(event);
  HeapifyUp(min_heap_.size() - 1);
}

Event* EventQueue::GetNextEvent() {
  if (min_heap_.empty()) {
    return nullptr;
  }
  
  Event* result = min_heap_[0];
  min_heap_[0] = min_heap_.back();
  min_heap_.pop_back();
  
  if (!min_heap_.empty()) {
    HeapifyDown(0);
  }
  
  return result;
}

bool EventQueue::ContainsEventOfType(const std::string& type) {
  for (Event* event : min_heap_) {
    if (event->type() == type) {
      return true;
    }
  }
  return false;
}

bool EventQueue::ContainsEventOfTypeAndHrid(const std::string& type, 
                                           const std::string& hrid) {
  for (Event* event : min_heap_) {
    if (event->type() == type && event->hrid() == hrid) {
      return true;
    }
  }
  return false;
}

void EventQueue::Clear() {
  // Delete all events and clear the heap
  for (Event* event : min_heap_) {
    delete event;
  }
  min_heap_.clear();
}

void EventQueue::ClearEventsForUnit(void* unit) {
  ClearMatching([unit](Event* event) {
    return event->source() == unit || event->target() == unit;
  });
}

void EventQueue::ClearEventsOfType(const std::string& type) {
  ClearMatching([&type](Event* event) {
    return event->type() == type;
  });
}

bool EventQueue::ClearMatching(std::function<bool(Event*)> fn) {
  bool cleared = false;
  std::vector<Event*> to_remove;
  
  // Find events to remove
  for (Event* event : min_heap_) {
    if (fn(event)) {
      to_remove.push_back(event);
      cleared = true;
    }
  }
  
  // Remove events
  for (Event* event : to_remove) {
    auto it = std::find(min_heap_.begin(), min_heap_.end(), event);
    if (it != min_heap_.end()) {
      min_heap_.erase(it);
      delete event;
    }
  }
  
  // Rebuild the heap
  if (cleared && !min_heap_.empty()) {
    std::make_heap(min_heap_.begin(), min_heap_.end(), 
        [this](Event* a, Event* b) { return !CompareEvents(a, b); });
  }
  
  return cleared;
}

Event* EventQueue::GetMatching(std::function<bool(Event*)> fn) {
  for (Event* event : min_heap_) {
    if (fn(event)) {
      return event;
    }
  }
  return nullptr;
}