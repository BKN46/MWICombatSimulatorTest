// combat_utilities.h - Utility functions for combat simulation.
// Copyright 2025 BKNMWICombatSimulator

#ifndef COMBAT_SIMULATOR_OVERHAUL_INCLUDE_COMBAT_UTILITIES_H_
#define COMBAT_SIMULATOR_OVERHAUL_INCLUDE_COMBAT_UTILITIES_H_

#include <memory>
#include <vector>
#include <unordered_map>
#include "combat_unit.h"
#include "ability.h"

namespace combat_simulator {

class CombatUtilities {
 public:
  // Returns the first living enemy in the list, or nullptr if none.
  static std::shared_ptr<CombatUnit> GetTarget(const std::vector<std::shared_ptr<CombatUnit>>& enemies);

  // Returns a random integer in [min, max] (inclusive, handles double input).
  static int RandomInt(double min, double max);

  // Processes an attack and returns the result (damage, crit, etc.).
  struct AttackResult {
    int damage_done = 0;
    bool did_hit = false;
    int reflect_damage_done = 0;
    std::string thorn_type;
    int life_steal_heal = 0;
    int hp_drain = 0;
    int mana_leech_mana = 0;
    // Experience fields for source and target
    std::unordered_map<std::string, double> experience_gained_source;
    std::unordered_map<std::string, double> experience_gained_target;
  };
  static AttackResult ProcessAttack(CombatUnit& source, CombatUnit& target, const AbilityEffect* ability_effect = nullptr);

  // Processes a heal and returns the amount healed.
  static int ProcessHeal(CombatUnit& source, const AbilityEffect& ability_effect, CombatUnit& target);

  // Processes a revive and returns the amount healed.
  static int ProcessRevive(CombatUnit& source, const AbilityEffect& ability_effect, CombatUnit& target);

  // Processes HP spend and returns the amount spent.
  static int ProcessSpendHp(CombatUnit& source, const AbilityEffect& ability_effect);

  // Tick value calculation for HoT/DoT.
  static int CalculateTickValue(int total_value, int total_ticks, int current_tick);

  // Experience calculation helpers.
  static double CalculateStaminaExperience(double damage_prevented, double damage_taken);
  static double CalculateIntelligenceExperience(double mana_used);
  static double CalculateAttackExperience(double damage, double damage_prevented, const std::string& combat_style);
  static double CalculatePowerExperience(double damage, double damage_prevented, const std::string& combat_style);
  static double CalculateDefenseExperience(double damage_prevented);
  static double CalculateRangedExperience(double damage, double damage_prevented);
  static double CalculateMagicExperience(double damage, double damage_prevented);
  static double CalculateHealingExperience(double healed);

  // ...other utility methods to be declared as needed...
};

}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_OVERHAUL_INCLUDE_COMBAT_UTILITIES_H_
