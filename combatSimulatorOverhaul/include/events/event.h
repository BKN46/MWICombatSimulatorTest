#ifndef COMBAT_SIMULATOR_OVERHAUL_INCLUDE_EVENT_H_
#define COMBAT_SIMULATOR_OVERHAUL_INCLUDE_EVENT_H_

#include <string>

class Event {
 public:
  // 构造函数
  Event(double time, const std::string& type,
              const std::string& hrid = "",
              void* source = nullptr,
              void* target = nullptr);
  virtual ~Event() = default;

  // 访问器
  double time() const { return time_; }
  const std::string& type() const { return type_; }
  const std::string& hrid() const { return hrid_; }
  void* source() const { return source_; }
  void* target() const { return target_; }

 protected:
  double time_;
  std::string type_;
  std::string hrid_;
  void* source_;
  void* target_;
};

#endif  // COMBAT_SIMULATOR_OVERHAUL_INCLUDE_EVENT_H_
