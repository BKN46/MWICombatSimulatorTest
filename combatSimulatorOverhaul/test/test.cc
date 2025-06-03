#include "../include/events/event_queue.h"
#include "../include/events/event.h"
#include <cassert>
#include <iostream>

void TestEventQueueBasic() {
  EventQueue queue;
  // 添加事件
  Event* e1 = new Event(1.0, "A");
  Event* e2 = new Event(0.5, "B");
  Event* e3 = new Event(2.0, "A", "hrid1");
  queue.AddEvent(e1);
  queue.AddEvent(e2);
  queue.AddEvent(e3);

  // 测试最小堆顺序
  Event* first = queue.GetNextEvent();
  assert(first->time() == 0.5 && first->type() == "B");
  delete first;
  Event* second = queue.GetNextEvent();
  assert(second->time() == 1.0 && second->type() == "A");
  delete second;
  Event* third = queue.GetNextEvent();
  assert(third->time() == 2.0 && third->type() == "A" && third->hrid() == "hrid1");
  delete third;
  assert(queue.GetNextEvent() == nullptr);
}

void TestEventQueueContains() {
  EventQueue queue;
  queue.AddEvent(new Event(1.0, "A"));
  queue.AddEvent(new Event(2.0, "B", "hrid2"));
  assert(queue.ContainsEventOfType("A"));
  assert(queue.ContainsEventOfType("B"));
  assert(!queue.ContainsEventOfType("C"));
  assert(queue.ContainsEventOfTypeAndHrid("B", "hrid2"));
  assert(!queue.ContainsEventOfTypeAndHrid("A", "hrid2"));
  queue.Clear();
}

int main() {
  TestEventQueueBasic();
  TestEventQueueContains();
  std::cout << "EventQueue 测试通过！" << std::endl;
  return 0;
}

