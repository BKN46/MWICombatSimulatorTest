// Copyright 2025 BKNMWICombatSimulator
//
// drops.h - Definition of the Drops class which represents item drop configuration
// in the combat system.

#ifndef COMBAT_SIMULATOR_DROPS_H_
#define COMBAT_SIMULATOR_DROPS_H_

#include <string>

namespace combat_simulator {

// Represents an item drop configuration in the combat system
class Drops {
 public:
  // Default constructor
  Drops() = default;
  
  // Constructs a Drops object with the specified parameters
  Drops(const std::string& item_hrid, 
       double drop_rate, 
       int min_count, 
       int max_count, 
       int elite_tier);
  
  // Getters
  const std::string& item_hrid() const { return item_hrid_; }
  double drop_rate() const { return drop_rate_; }
  int min_count() const { return min_count_; }
  int max_count() const { return max_count_; }
  int elite_tier() const { return elite_tier_; }
  
  // Setters
  void set_item_hrid(const std::string& item_hrid) { item_hrid_ = item_hrid; }
  void set_drop_rate(double drop_rate) { drop_rate_ = drop_rate; }
  void set_min_count(int min_count) { min_count_ = min_count; }
  void set_max_count(int max_count) { max_count_ = max_count; }
  void set_elite_tier(int elite_tier) { elite_tier_ = elite_tier; }
  
 private:
  std::string item_hrid_;  // The HRID (human-readable ID) of the dropped item
  double drop_rate_ = 0.0;  // The probability of the item dropping (0.0-1.0)
  int min_count_ = 0;       // The minimum number of items that can drop
  int max_count_ = 0;       // The maximum number of items that can drop
  int elite_tier_ = 0;      // The elite tier requirement for this drop
};

}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_DROPS_H_
