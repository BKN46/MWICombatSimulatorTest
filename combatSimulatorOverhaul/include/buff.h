// Copyright 2025 BKNMWICombatSimulator
//
// buff.h - Definition of the Buff class which represents a buff/effect
// that can be applied to combat units.

#ifndef COMBAT_SIMULATOR_BUFF_H_
#define COMBAT_SIMULATOR_BUFF_H_

#include <string>

namespace combat_simulator {

// Represents a temporary or permanent buff that can be applied to a combat unit.
// Buffs can provide ratio-based or flat boosts to various stats.
class Buff {
 public:
  // Constructs a buff with the provided parameters
  Buff() = default;
  
  // Constructs a buff with the provided parameters and applies level scaling
  Buff(const Buff& buff, int level = 1);
  
  // Default destructor
  ~Buff() = default;

  // Getter methods
  const std::string& unique_hrid() const { return unique_hrid_; }
  const std::string& type_hrid() const { return type_hrid_; }
  double ratio_boost() const { return ratio_boost_; }
  double flat_boost() const { return flat_boost_; }
  int64_t duration() const { return duration_; }
  int64_t start_time() const { return start_time_; }
  
  // Setter methods
  void set_unique_hrid(const std::string& unique_hrid) { unique_hrid_ = unique_hrid; }
  void set_type_hrid(const std::string& type_hrid) { type_hrid_ = type_hrid; }
  void set_ratio_boost(double ratio_boost) { ratio_boost_ = ratio_boost; }
  void set_flat_boost(double flat_boost) { flat_boost_ = flat_boost; }
  void set_duration(int64_t duration) { duration_ = duration; }
  void set_start_time(int64_t start_time) { start_time_ = start_time; }

 public:  // Normally private, but kept public for direct access in combat_unit.cc
  std::string unique_hrid_;    // Unique identifier for this specific buff
  std::string type_hrid_;      // Type identifier for this buff category
  double ratio_boost_ = 0.0;   // Percentage-based boost
  double ratio_boost_level_bonus_ = 0.0;  // Additional ratio boost per level
  double flat_boost_ = 0.0;    // Flat value boost
  double flat_boost_level_bonus_ = 0.0;   // Additional flat boost per level
  int64_t start_time_ = 0;     // When the buff was applied
  int64_t duration_ = 0;       // How long the buff lasts
};

}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_BUFF_H_
