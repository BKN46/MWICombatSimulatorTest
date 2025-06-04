// Copyright 2025 BKNMWICombatSimulator
//
// monster.h - Definition of the Monster class which represents a combat monster 
// entity in the simulation system.

#ifndef COMBAT_SIMULATOR_MONSTER_H_
#define COMBAT_SIMULATOR_MONSTER_H_

#include <memory>
#include <string>
#include <vector>

#include "ability.h"
#include "combat_unit.h"
#include "drops.h"
#include "nlohmann/json.hpp"

namespace combat_simulator {

// Represents a monster in the combat system that inherits from CombatUnit
class Monster : public CombatUnit {
 public:
  // Default constructor
  Monster() = default;
  
  // Constructs a Monster with the specified HRID and elite tier
  Monster(const std::string& hrid, int elite_tier = 0);
  
  // Updates combat details based on monster specific stats and elite tier
  void UpdateCombatDetails() override;
  
  // Getter for elite tier
  int elite_tier() const { return elite_tier_; }
  
  // Setter for elite tier
  void set_elite_tier(int elite_tier) { elite_tier_ = elite_tier; }
  
  // 允许访问abilities_
  std::vector<std::shared_ptr<Ability>>& abilities() { return abilities_; }
  const std::vector<std::shared_ptr<Ability>>& abilities() const { return abilities_; }
  
 private:
  // Helper function to load monster detail data from JSON
  static nlohmann::json LoadCombatMonsterDetailMap();
  
  // Member variables
  int elite_tier_ = 0;
};

}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_MONSTER_H_
