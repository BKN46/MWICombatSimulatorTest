#ifndef COMBAT_SIMULATOR_OVERHAUL_INCLUDE_EVENT_QUEUE_H_
#define COMBAT_SIMULATOR_OVERHAUL_INCLUDE_EVENT_QUEUE_H_

#include <functional>
#include <algorithm>
#include <string>
#include <vector>

// Forward declaration for the Event class
class Event;

class EventQueue {
 public:
  EventQueue();
  ~EventQueue();
  
  // Disallow copying and assignment
  EventQueue(const EventQueue&) = delete;
  EventQueue& operator=(const EventQueue&) = delete;
  
  void AddEvent(Event* event);
  Event* GetNextEvent();
  bool ContainsEventOfType(const std::string& type);
  bool ContainsEventOfTypeAndHrid(const std::string& type, const std::string& hrid);
  void Clear();
  void ClearEventsForUnit(void* unit);
  void ClearEventsOfType(const std::string& type);
  bool ClearMatching(std::function<bool(Event*)> fn);
  Event* GetMatching(std::function<bool(Event*)> fn);

 private:
  std::vector<Event*> min_heap_;
  
  // Helper methods for heap operations
  void HeapifyUp(size_t index);
  void HeapifyDown(size_t index);
  bool CompareEvents(Event* a, Event* b) const;
};

#endif  // COMBAT_SIMULATOR_OVERHAUL_INCLUDE_EVENT_QUEUE_H_
