// Copyright 2025 BKNMWICombatSimulator
//
// consumable.h - Definition of the Consumable class which represents
// consumable items that can be used by combat units.

#ifndef COMBAT_SIMULATOR_CONSUMABLE_H_
#define COMBAT_SIMULATOR_CONSUMABLE_H_

#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "buff.h"
#include "trigger.h"
#include "nlohmann/json.hpp"

namespace combat_simulator {

// Forward declarations
class CombatUnit;

// Represents a consumable item that can be used in combat
class Consumable {
 public:
  // Default constructor
  Consumable() = default;
  
  // Constructs a consumable with the specified HRID and optional triggers
  Consumable(const std::string& hrid, 
            const std::vector<std::shared_ptr<Trigger>>& triggers = {});
  
  // Creates a consumable from a data transfer object (JSON)
  static std::unique_ptr<Consumable> CreateFromDTO(const nlohmann::json& dto);
  
  // Checks if the consumable should trigger based on combat conditions
  bool ShouldTrigger(int64_t current_time, 
                    const CombatUnit& source,
                    const CombatUnit* target,
                    const std::vector<std::shared_ptr<CombatUnit>>& friendlies,
                    const std::vector<std::shared_ptr<CombatUnit>>& enemies) const;
  
  // Getters
  const std::string& hrid() const { return hrid_; }
  int64_t cooldown_duration() const { return cooldown_duration_; }
  double hitpoint_restore() const { return hitpoint_restore_; }
  double manapoint_restore() const { return manapoint_restore_; }
  int64_t recovery_duration() const { return recovery_duration_; }
  const std::string& category_hrid() const { return category_hrid_; }
  const std::vector<Buff>& buffs() const { return buffs_; }
  const std::vector<std::shared_ptr<Trigger>>& triggers() const { return triggers_; }
  int64_t last_used() const { return last_used_; }
  
  // Setters
  void set_last_used(int64_t time) { last_used_ = time; }
  void SetLastUsed(int64_t time) { last_used_ = time; }
  
 private:
  // Helper function to load item detail map
  static nlohmann::json LoadItemDetailMap();
  
  // Member variables
  std::string hrid_;
  int64_t cooldown_duration_ = 0;
  double hitpoint_restore_ = 0.0;
  double manapoint_restore_ = 0.0;
  int64_t recovery_duration_ = 0;
  std::string category_hrid_;
  std::vector<Buff> buffs_;
  std::vector<std::shared_ptr<Trigger>> triggers_;
  int64_t last_used_ = std::numeric_limits<int64_t>::min();
};

}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_CONSUMABLE_H_
