// Copyright 2025 BKNMWICombatSimulator
//
// combat_unit.h - Definition of the CombatUnit class which represents a basic
// combat entity in the simulation system.

#ifndef COMBAT_SIMULATOR_COMBAT_UNIT_H_
#define COMBAT_SIMULATOR_COMBAT_UNIT_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "buff.h"
#include "house_room.h"
#include "nlohmann/json.hpp"

namespace combat_simulator {

// Forward declarations
class Ability;
class Consumable;

// Represents combat statistics
struct CombatStats {
  std::string combat_style_hrid = "/combat_styles/smash";
  std::string damage_type = "/damage_types/physical";
  int64_t attack_interval = 3000000000;
  double auto_attack_damage = 0.0;
  double critical_rate = 0.0;
  double critical_damage = 0.0;
  double stab_accuracy = 0.0;
  double slash_accuracy = 0.0;
  double smash_accuracy = 0.0;
  double ranged_accuracy = 0.0;
  double magic_accuracy = 0.0;
  double stab_damage = 0.0;
  double slash_damage = 0.0;
  double smash_damage = 0.0;
  double ranged_damage = 0.0;
  double magic_damage = 0.0;
  double task_damage = 0.0;
  double physical_amplify = 0.0;
  double water_amplify = 0.0;
  double nature_amplify = 0.0;
  double fire_amplify = 0.0;
  double healing_amplify = 0.0;
  double physical_thorns = 0.0;
  double elemental_thorns = 0.0;
  double max_hitpoints = 0.0;
  double max_manapoints = 0.0;
  double stab_evasion = 0.0;
  double slash_evasion = 0.0;
  double smash_evasion = 0.0;
  double ranged_evasion = 0.0;
  double magic_evasion = 0.0;
  double armor = 0.0;
  double water_resistance = 0.0;
  double nature_resistance = 0.0;
  double fire_resistance = 0.0;
  double life_steal = 0.0;
  double hp_regen_per_10 = 0.01;
  double mp_regen_per_10 = 0.01;
  double combat_drop_rate = 0.0;
  double combat_drop_quantity = 0.0;
  double combat_rare_find = 0.0;
  double combat_experience = 0.0;
  int food_slots = 1;
  int drink_slots = 1;
  double armor_penetration = 0.0;
  double water_penetration = 0.0;
  double nature_penetration = 0.0;
  double fire_penetration = 0.0;
  double mana_leech = 0.0;
  double cast_speed = 0.0;
  double threat = 100.0;
  double parry = 0.0;
  double mayhem = 0.0;
  double pierce = 0.0;
  double curse = 0.0;
  double ripple = 0.0;
  double bloom = 0.0;
  double blaze = 0.0;
  double weaken = 0.0;
  double fury = 0.0;
  double food_haste = 0.0;
  double drink_concentration = 0.0;
  double damage_taken = 0.0;
  double attack_speed = 0.0;
  double armor_damage_ratio = 0.0;
  double hp_drain_ratio = 0.0;
  double ability_haste = 0.0;
  double tenacity = 0.0;

  // Sets a stat by name
  void SetStat(const std::string& stat_name, double value);
  
  // Gets a stat by name
  double GetStat(const std::string& stat_name) const;
};

// Represents the complete combat details of a unit
struct CombatDetails {
  int stamina_level = 1;
  int intelligence_level = 1;
  int attack_level = 1;
  int power_level = 1;
  int defense_level = 1;
  int ranged_level = 1;
  int magic_level = 1;
  double max_hitpoints = 110.0;
  double current_hitpoints = 110.0;
  double max_manapoints = 110.0;
  double current_manapoints = 110.0;
  double stab_accuracy_rating = 11.0;
  double slash_accuracy_rating = 11.0;
  double smash_accuracy_rating = 11.0;
  double ranged_accuracy_rating = 11.0;
  double magic_accuracy_rating = 11.0;
  double stab_max_damage = 11.0;
  double slash_max_damage = 11.0;
  double smash_max_damage = 11.0;
  double ranged_max_damage = 11.0;
  double magic_max_damage = 11.0;
  double stab_evasion_rating = 11.0;
  double slash_evasion_rating = 11.0;
  double smash_evasion_rating = 11.0;
  double ranged_evasion_rating = 11.0;
  double magic_evasion_rating = 11.0;
  double total_armor = 0.2;
  double total_water_resistance = 0.4;
  double total_nature_resistance = 0.4;
  double total_fire_resistance = 0.4;
  double ability_haste = 0.0;
  double tenacity = 0.0;
  double total_threat = 100.0;
  CombatStats combat_stats;
};

// CombatUnit represents a basic combat entity in the simulation.
class CombatUnit {
 public:
  // Constructor
  CombatUnit();
  
  // Virtual destructor
  virtual ~CombatUnit() = default;
  
  // Updates combat details based on current stats and buffs
  virtual void UpdateCombatDetails();
  
  // Adds a curse effect to the unit
  void AddCurse(double curse);
  
  // Updates fury value based on hit success
  double UpdateFury(bool is_hit, double fury);
  
  // Adds a temporary buff
  void AddBuff(const Buff& buff, int64_t current_time);
  
  // Removes a specific buff
  void RemoveBuff(const Buff& buff);
  
  // Adds a permanent buff
  void AddPermanentBuff(const Buff& buff);
  
  // Generates permanent buffs from house rooms and zone
  void GeneratePermanentBuffs();
  
  // Removes expired buffs
  void RemoveExpiredBuffs(int64_t current_time);
  
  // Clears all temporary buffs
  void ClearBuffs();
  
  // Clears all crowd control effects
  void ClearCCs();
  
  // Gets buffs of a specific type
  std::vector<Buff> GetBuffBoosts(const std::string& type);
  
  // Gets accumulated buff effects of a specific type
  Buff GetBuffBoost(const std::string& type);
  
  // Resets the unit to its initial state
  virtual void Reset(int64_t current_time = 0);
  
  // Resets all cooldowns
  void ResetCooldowns(int64_t current_time = 0);
  
  // Adds hitpoints to the unit (limited by max)
  double AddHitpoints(double hitpoints);
  
  // Adds manapoints to the unit (limited by max)
  double AddManapoints(double manapoints);
  
  // Getter methods
  const CombatDetails& combat_details() const { return combat_details_; }
  const std::map<std::string, Buff>& combat_buffs() const { return combat_buffs_; }
  const std::map<std::string, Buff>& permanent_buffs() const { return permanent_buffs_; }
  const std::vector<Buff>& zone_buffs() const { return zone_buffs_; }
  bool is_player() const { return is_player_; }
  bool is_stunned() const { return is_stunned_; }
  int64_t stun_expire_time() const { return stun_expire_time_; }
  bool is_blinded() const { return is_blinded_; }
  int64_t blind_expire_time() const { return blind_expire_time_; }
  bool is_silenced() const { return is_silenced_; }
  int64_t silence_expire_time() const { return silence_expire_time_; }
  double curse_value() const { return curse_value_; }
  double fury_value() const { return fury_value_; }
  bool is_weakened() const { return is_weakened_; }
  int64_t weaken_expire_time() const { return weaken_expire_time_; }
  double weaken_percentage() const { return weaken_percentage_; }
  const std::string& hrid() const { return hrid_; }
  
 protected:
  bool is_player_ = false;
  bool is_stunned_ = false;
  int64_t stun_expire_time_ = 0;
  bool is_blinded_ = false;
  int64_t blind_expire_time_ = 0;
  bool is_silenced_ = false;
  int64_t silence_expire_time_ = 0;
  double curse_value_ = 0.0;
  double fury_value_ = 0.0;
  bool is_weakened_ = false;
  int64_t weaken_expire_time_ = 0;
  double weaken_percentage_ = 0.0;
  
  // Base levels
  int stamina_level_ = 1;
  int intelligence_level_ = 1;
  int attack_level_ = 1;
  int power_level_ = 1;
  int defense_level_ = 1;
  int ranged_level_ = 1;
  int magic_level_ = 1;
  
  // Equipment and abilities
  std::vector<std::shared_ptr<Ability>> abilities_;
  std::vector<std::shared_ptr<Consumable>> food_;
  std::vector<std::shared_ptr<Consumable>> drinks_;
  std::vector<HouseRoom> house_rooms_;
  std::vector<nlohmann::json> drop_table_;
  std::vector<nlohmann::json> rare_drop_table_;
  std::map<std::string, double> ability_mana_costs_;
  
  // Combat details
  CombatDetails combat_details_;
  
  // Buffs
  std::map<std::string, Buff> combat_buffs_;
  std::map<std::string, Buff> permanent_buffs_;
  std::vector<Buff> zone_buffs_;
  
  // Unit identification
  std::string hrid_ = "";
};

}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_COMBAT_UNIT_H_
