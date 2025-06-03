// Copyright 2025 BKNMWICombatSimulator
//
// ability.h - Definition of the Ability class which represents an ability
// that can be used by combat units in the combat system.

#ifndef COMBAT_SIMULATOR_ABILITY_H_
#define COMBAT_SIMULATOR_ABILITY_H_

#include <memory>
#include <string>
#include <vector>

#include "buff.h"
#include "trigger.h"
#include "nlohmann/json.hpp"

namespace combat_simulator {

// Forward declarations
class CombatUnit;

// Represents an ability effect in combat
struct AbilityEffect {
  std::string target_type;
  std::string effect_type;
  std::string combat_style_hrid;
  std::string damage_type;
  double damage_flat = 0.0;
  double damage_ratio = 0.0;
  double bonus_accuracy_ratio = 0.0;
  double damage_over_time_ratio = 0.0;
  int64_t damage_over_time_duration = 0;
  double armor_damage_ratio = 0.0;
  double hp_drain_ratio = 0.0;
  double pierce_chance = 0.0;
  double blind_chance = 0.0;
  int64_t blind_duration = 0;
  double silence_chance = 0.0;
  int64_t silence_duration = 0;
  double stun_chance = 0.0;
  int64_t stun_duration = 0;
  double spend_hp_ratio = 0.0;
  std::vector<Buff> buffs;
};

// Represents an ability that can be used by combat units
class Ability {
 public:
  // Default constructor
  Ability() = default;
  
  // Constructs an ability with the specified HRID and level
  Ability(const std::string& hrid, int level = 1, 
         const std::vector<std::shared_ptr<Trigger>>& triggers = {});
  
  // Creates an ability from a data transfer object (JSON)
  static std::unique_ptr<Ability> CreateFromDTO(const nlohmann::json& dto);
  
  // Checks if the ability should trigger based on combat conditions
  bool ShouldTrigger(int64_t current_time, 
                    const CombatUnit& source,
                    const CombatUnit* target,
                    const std::vector<std::shared_ptr<CombatUnit>>& friendlies,
                    const std::vector<std::shared_ptr<CombatUnit>>& enemies) const;
  
  // Getters
  const std::string& hrid() const { return hrid_; }
  int level() const { return level_; }
  int64_t mana_cost() const { return mana_cost_; }
  int64_t cooldown_duration() const { return cooldown_duration_; }
  int64_t cast_duration() const { return cast_duration_; }
  bool is_special_ability() const { return is_special_ability_; }
  const std::vector<AbilityEffect>& ability_effects() const { return ability_effects_; }
  const std::vector<std::shared_ptr<Trigger>>& triggers() const { return triggers_; }
  int64_t last_used() const { return last_used_; }
  
  // Setters
  void set_last_used(int64_t time) { last_used_ = time; }
  void SetLastUsed(int64_t time) { last_used_ = time; }
  int64_t GetCooldownDuration() const { return cooldown_duration_; }
  
 private:
  // Helper function to load ability data from JSON
  static nlohmann::json LoadAbilityDetailMap();
  
  // Helper function to load combat stat abilities
  static nlohmann::json LoadAbilityFromCombatStat();
  
  // Member variables
  std::string hrid_;
  int level_ = 1;
  int64_t mana_cost_ = 0;
  int64_t cooldown_duration_ = 0;
  int64_t cast_duration_ = 0;
  bool is_special_ability_ = false;
  std::vector<AbilityEffect> ability_effects_;
  std::vector<std::shared_ptr<Trigger>> triggers_;
  int64_t last_used_ = std::numeric_limits<int64_t>::min();
};

}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_ABILITY_H_
