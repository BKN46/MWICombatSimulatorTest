// Copyright 2025 BKNMWICombatSimulator
//
// Implementation of the CombatUnit class for the combat simulator.

#include "combat_unit.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "ability.h"
#include "consumable.h"
#include "house_room.h"

namespace combat_simulator {

void CombatStats::SetStat(const std::string& stat_name, double value) {
  if (stat_name == "stabAccuracy") stab_accuracy = value;
  else if (stat_name == "slashAccuracy") slash_accuracy = value;
  else if (stat_name == "smashAccuracy") smash_accuracy = value;
  else if (stat_name == "rangedAccuracy") ranged_accuracy = value;
  else if (stat_name == "magicAccuracy") magic_accuracy = value;
  else if (stat_name == "stabDamage") stab_damage = value;
  else if (stat_name == "slashDamage") slash_damage = value;
  else if (stat_name == "smashDamage") smash_damage = value;
  else if (stat_name == "rangedDamage") ranged_damage = value;
  else if (stat_name == "magicDamage") magic_damage = value;
  else if (stat_name == "taskDamage") task_damage = value;
  else if (stat_name == "physicalAmplify") physical_amplify = value;
  else if (stat_name == "waterAmplify") water_amplify = value;
  else if (stat_name == "natureAmplify") nature_amplify = value;
  else if (stat_name == "fireAmplify") fire_amplify = value;
  else if (stat_name == "healingAmplify") healing_amplify = value;
  else if (stat_name == "stabEvasion") stab_evasion = value;
  else if (stat_name == "slashEvasion") slash_evasion = value;
  else if (stat_name == "smashEvasion") smash_evasion = value;
  else if (stat_name == "rangedEvasion") ranged_evasion = value;
  else if (stat_name == "magicEvasion") magic_evasion = value;
  else if (stat_name == "armor") armor = value;
  else if (stat_name == "waterResistance") water_resistance = value;
  else if (stat_name == "natureResistance") nature_resistance = value;
  else if (stat_name == "fireResistance") fire_resistance = value;
  else if (stat_name == "maxHitpoints") max_hitpoints = value;
  else if (stat_name == "maxManapoints") max_manapoints = value;
  else if (stat_name == "lifeSteal") life_steal = value;
  else if (stat_name == "hpRegenPer10") hp_regen_per_10 = value;
  else if (stat_name == "mpRegenPer10") mp_regen_per_10 = value;
  else if (stat_name == "physicalThorns") physical_thorns = value;
  else if (stat_name == "elementalThorns") elemental_thorns = value;
  else if (stat_name == "combatDropRate") combat_drop_rate = value;
  else if (stat_name == "combatRareFind") combat_rare_find = value;
  else if (stat_name == "combatDropQuantity") combat_drop_quantity = value;
  else if (stat_name == "combatExperience") combat_experience = value;
  else if (stat_name == "criticalRate") critical_rate = value;
  else if (stat_name == "criticalDamage") critical_damage = value;
  else if (stat_name == "armorPenetration") armor_penetration = value;
  else if (stat_name == "waterPenetration") water_penetration = value;
  else if (stat_name == "naturePenetration") nature_penetration = value;
  else if (stat_name == "firePenetration") fire_penetration = value;
  else if (stat_name == "abilityHaste") ability_haste = value;
  else if (stat_name == "tenacity") tenacity = value;
  else if (stat_name == "manaLeech") mana_leech = value;
  else if (stat_name == "castSpeed") cast_speed = value;
  else if (stat_name == "threat") threat = value;
  else if (stat_name == "parry") parry = value;
  else if (stat_name == "mayhem") mayhem = value;
  else if (stat_name == "pierce") pierce = value;
  else if (stat_name == "curse") curse = value;
  else if (stat_name == "fury") fury = value;
  else if (stat_name == "weaken") weaken = value;
  else if (stat_name == "ripple") ripple = value;
  else if (stat_name == "bloom") bloom = value;
  else if (stat_name == "blaze") blaze = value;
  else if (stat_name == "attackSpeed") attack_speed = value;
  else if (stat_name == "foodHaste") food_haste = value;
  else if (stat_name == "drinkConcentration") drink_concentration = value;
  else if (stat_name == "autoAttackDamage") auto_attack_damage = value;
  else if (stat_name == "foodSlots") food_slots = static_cast<int>(value);
  else if (stat_name == "drinkSlots") drink_slots = static_cast<int>(value);
  // Add other stats as needed
}

double CombatStats::GetStat(const std::string& stat_name) const {
  if (stat_name == "stabAccuracy") return stab_accuracy;
  else if (stat_name == "slashAccuracy") return slash_accuracy;
  else if (stat_name == "smashAccuracy") return smash_accuracy;
  else if (stat_name == "rangedAccuracy") return ranged_accuracy;
  else if (stat_name == "magicAccuracy") return magic_accuracy;
  else if (stat_name == "stabDamage") return stab_damage;
  else if (stat_name == "slashDamage") return slash_damage;
  else if (stat_name == "smashDamage") return smash_damage;
  else if (stat_name == "rangedDamage") return ranged_damage;
  else if (stat_name == "magicDamage") return magic_damage;
  else if (stat_name == "taskDamage") return task_damage;
  else if (stat_name == "physicalAmplify") return physical_amplify;
  else if (stat_name == "waterAmplify") return water_amplify;
  else if (stat_name == "natureAmplify") return nature_amplify;
  else if (stat_name == "fireAmplify") return fire_amplify;
  else if (stat_name == "healingAmplify") return healing_amplify;
  else if (stat_name == "stabEvasion") return stab_evasion;
  else if (stat_name == "slashEvasion") return slash_evasion;
  else if (stat_name == "smashEvasion") return smash_evasion;
  else if (stat_name == "rangedEvasion") return ranged_evasion;
  else if (stat_name == "magicEvasion") return magic_evasion;
  else if (stat_name == "armor") return armor;
  else if (stat_name == "waterResistance") return water_resistance;
  else if (stat_name == "natureResistance") return nature_resistance;
  else if (stat_name == "fireResistance") return fire_resistance;
  else if (stat_name == "maxHitpoints") return max_hitpoints;
  else if (stat_name == "maxManapoints") return max_manapoints;
  else if (stat_name == "lifeSteal") return life_steal;
  else if (stat_name == "hpRegenPer10") return hp_regen_per_10;
  else if (stat_name == "mpRegenPer10") return mp_regen_per_10;
  else if (stat_name == "physicalThorns") return physical_thorns;
  else if (stat_name == "elementalThorns") return elemental_thorns;
  else if (stat_name == "combatDropRate") return combat_drop_rate;
  else if (stat_name == "combatRareFind") return combat_rare_find;
  else if (stat_name == "combatDropQuantity") return combat_drop_quantity;
  else if (stat_name == "combatExperience") return combat_experience;
  else if (stat_name == "criticalRate") return critical_rate;
  else if (stat_name == "criticalDamage") return critical_damage;
  else if (stat_name == "armorPenetration") return armor_penetration;
  else if (stat_name == "waterPenetration") return water_penetration;
  else if (stat_name == "naturePenetration") return nature_penetration;
  else if (stat_name == "firePenetration") return fire_penetration;
  else if (stat_name == "abilityHaste") return ability_haste;
  else if (stat_name == "tenacity") return tenacity;
  else if (stat_name == "manaLeech") return mana_leech;
  else if (stat_name == "castSpeed") return cast_speed;
  else if (stat_name == "threat") return threat;
  else if (stat_name == "parry") return parry;
  else if (stat_name == "mayhem") return mayhem;
  else if (stat_name == "pierce") return pierce;
  else if (stat_name == "curse") return curse;
  else if (stat_name == "fury") return fury;
  else if (stat_name == "weaken") return weaken;
  else if (stat_name == "ripple") return ripple;
  else if (stat_name == "bloom") return bloom;
  else if (stat_name == "blaze") return blaze;
  else if (stat_name == "attackSpeed") return attack_speed;
  else if (stat_name == "foodHaste") return food_haste;
  else if (stat_name == "drinkConcentration") return drink_concentration;
  else if (stat_name == "autoAttackDamage") return auto_attack_damage;
  else if (stat_name == "foodSlots") return static_cast<double>(food_slots);
  else if (stat_name == "drinkSlots") return static_cast<double>(drink_slots);
  // Add other stats as needed
  return 0.0;
}

CombatUnit::CombatUnit() {
  // Initialize abilities, food, and drinks vectors with nullptrs
  abilities_.resize(4, nullptr);
  food_.resize(3, nullptr);
  drinks_.resize(3, nullptr);
}

void CombatUnit::UpdateCombatDetails() {
  // Set base regeneration rates for players
  if (is_player_) {
    if (combat_details_.combat_stats.hp_regen_per_10 == 0.0) {
      combat_details_.combat_stats.hp_regen_per_10 = 0.01;
    } else {
      combat_details_.combat_stats.hp_regen_per_10 = 0.01 + combat_details_.combat_stats.hp_regen_per_10;
    }
    
    if (combat_details_.combat_stats.mp_regen_per_10 == 0.0) {
      combat_details_.combat_stats.mp_regen_per_10 = 0.01;
    } else {
      combat_details_.combat_stats.mp_regen_per_10 = 0.01 + combat_details_.combat_stats.mp_regen_per_10;
    }
  }
  
  // Update base stats from buffs
  std::vector<std::string> stats = {"stamina", "intelligence", "attack", "power", "defense", "ranged", "magic"};
  for (const auto& stat : stats) {
    double level = stat == "stamina" ? combat_details_.stamina_level :
                   stat == "intelligence" ? combat_details_.intelligence_level :
                   stat == "attack" ? combat_details_.attack_level :
                   stat == "power" ? combat_details_.power_level :
                   stat == "defense" ? combat_details_.defense_level :
                   stat == "ranged" ? combat_details_.ranged_level :
                   combat_details_.magic_level;
    
    int base_level = stat == "stamina" ? stamina_level_ :
                     stat == "intelligence" ? intelligence_level_ :
                     stat == "attack" ? attack_level_ :
                     stat == "power" ? power_level_ :
                     stat == "defense" ? defense_level_ :
                     stat == "ranged" ? ranged_level_ :
                     magic_level_;
    
    level = base_level;
    std::vector<Buff> boosts = GetBuffBoosts("/buff_types/" + stat + "_level");
    for (const auto& buff : boosts) {
      level += std::floor(base_level * buff.ratio_boost());
      level += buff.flat_boost();
    }
  }
  
  // Calculate max hit points and mana points
  combat_details_.max_hitpoints = 
    10 * (10 + combat_details_.stamina_level) + combat_details_.combat_stats.max_hitpoints;
  combat_details_.max_manapoints = 
    10 * (10 + combat_details_.intelligence_level) + combat_details_.combat_stats.max_manapoints;
  
  // Get accuracy and damage boosts
  Buff accuracy_boost = GetBuffBoost("/buff_types/accuracy");
  Buff damage_boost = GetBuffBoost("/buff_types/damage");
  
  // Calculate accuracy and damage ratings for melee styles
  std::vector<std::string> melee_styles = {"stab", "slash", "smash"};
  for (const auto& style : melee_styles) {
    // Calculate accuracy rating
    double& accuracy_rating = 
      style == "stab" ? combat_details_.stab_accuracy_rating :
      style == "slash" ? combat_details_.slash_accuracy_rating :
      combat_details_.smash_accuracy_rating;
    
    accuracy_rating = (10 + combat_details_.attack_level) * 
      (1 + combat_details_.combat_stats.GetStat(style + "Accuracy")) *
      (1 + accuracy_boost.ratio_boost()) *
      (1 + fury_value_);
    
    // Calculate max damage
    double& max_damage = 
      style == "stab" ? combat_details_.stab_max_damage :
      style == "slash" ? combat_details_.slash_max_damage :
      combat_details_.smash_max_damage;
    
    max_damage = (10 + combat_details_.power_level) *
      (1 + combat_details_.combat_stats.GetStat(style + "Damage")) *
      (1 + damage_boost.ratio_boost()) *
      (1 + fury_value_);
    
    // Calculate evasion rating
    double& evasion_rating = 
      style == "stab" ? combat_details_.stab_evasion_rating :
      style == "slash" ? combat_details_.slash_evasion_rating :
      combat_details_.smash_evasion_rating;
    
    double base_evasion = (10 + combat_details_.defense_level) * 
      (1 + combat_details_.combat_stats.GetStat(style + "Evasion"));
    evasion_rating = base_evasion;
    
    std::vector<Buff> evasion_boosts = GetBuffBoosts("/buff_types/evasion");
    for (const auto& boost : evasion_boosts) {
      evasion_rating += boost.flat_boost();
      evasion_rating += base_evasion * boost.ratio_boost();
    }
  }
  
  // Calculate ranged accuracy and damage
  combat_details_.ranged_accuracy_rating = (10 + combat_details_.ranged_level) *
    (1 + combat_details_.combat_stats.ranged_accuracy) *
    (1 + accuracy_boost.ratio_boost()) *
    (1 + fury_value_);
  
  combat_details_.ranged_max_damage = (10 + combat_details_.ranged_level) *
    (1 + combat_details_.combat_stats.ranged_damage) *
    (1 + damage_boost.ratio_boost()) *
    (1 + fury_value_);
  
  // Calculate ranged evasion
  double base_ranged_evasion = (10 + combat_details_.defense_level) * 
    (1 + combat_details_.combat_stats.ranged_evasion);
  combat_details_.ranged_evasion_rating = base_ranged_evasion;
  
  std::vector<Buff> evasion_boosts = GetBuffBoosts("/buff_types/evasion");
  for (const auto& boost : evasion_boosts) {
    combat_details_.ranged_evasion_rating += boost.flat_boost();
    combat_details_.ranged_evasion_rating += base_ranged_evasion * boost.ratio_boost();
  }
  
  // Calculate damage taken from curse
  double base_damage_taken = curse_value_;
  combat_details_.combat_stats.damage_taken = base_damage_taken;
  std::vector<Buff> damage_taken_boosts = GetBuffBoosts("/buff_types/damage_taken");
  for (const auto& boost : damage_taken_boosts) {
    combat_details_.combat_stats.damage_taken += boost.flat_boost();
  }
  
  // Calculate magic accuracy and damage
  combat_details_.magic_accuracy_rating = (10 + combat_details_.magic_level) *
    (1 + combat_details_.combat_stats.magic_accuracy) *
    (1 + accuracy_boost.ratio_boost()) *
    (1 + fury_value_);
  
  combat_details_.magic_max_damage = (10 + combat_details_.magic_level) *
    (1 + combat_details_.combat_stats.magic_damage) *
    (1 + damage_boost.ratio_boost()) *
    (1 + fury_value_);
  
  // Calculate magic evasion
  double base_magic_evasion = (10 + (combat_details_.defense_level * 0.75 + 
    combat_details_.ranged_level * 0.25)) * (1 + combat_details_.combat_stats.magic_evasion);
  combat_details_.magic_evasion_rating = base_magic_evasion;
  
  for (const auto& boost : evasion_boosts) {
    combat_details_.magic_evasion_rating += boost.flat_boost();
    combat_details_.magic_evasion_rating += base_magic_evasion * boost.ratio_boost();
  }
  
  // Add elemental amplify buffs
  combat_details_.combat_stats.physical_amplify += 
    GetBuffBoost("/buff_types/physical_amplify").flat_boost();
  combat_details_.combat_stats.water_amplify += 
    GetBuffBoost("/buff_types/water_amplify").flat_boost();
  combat_details_.combat_stats.nature_amplify += 
    GetBuffBoost("/buff_types/nature_amplify").flat_boost();
  combat_details_.combat_stats.fire_amplify += 
    GetBuffBoost("/buff_types/fire_amplify").flat_boost();
  
  // Calculate attack interval based on attack level and speed
  if (is_player_) {
    combat_details_.combat_stats.attack_interval /= (1 + (combat_details_.attack_level / 2000.0));
  }
  
  double base_attack_speed = combat_details_.combat_stats.attack_speed;
  std::vector<Buff> attack_interval_boosts = GetBuffBoosts("/buff_types/attack_speed");
  double attack_interval_ratio_boost = 0.0;
  for (const auto& boost : attack_interval_boosts) {
    attack_interval_ratio_boost += boost.ratio_boost();
  }
  combat_details_.combat_stats.attack_interval /= (1 + (base_attack_speed + attack_interval_ratio_boost));
  
  // Calculate armor
  double base_armor = 0.2 * combat_details_.defense_level + combat_details_.combat_stats.armor;
  combat_details_.total_armor = base_armor;
  std::vector<Buff> armor_boosts = GetBuffBoosts("/buff_types/armor");
  for (const auto& boost : armor_boosts) {
    combat_details_.total_armor += boost.flat_boost();
    combat_details_.total_armor += base_armor * boost.ratio_boost();
  }
  
  // Calculate elemental resistances
  double base_water_resistance = 0.1 * (combat_details_.defense_level + combat_details_.magic_level) +
    combat_details_.combat_stats.water_resistance;
  combat_details_.total_water_resistance = base_water_resistance;
  std::vector<Buff> water_resistance_boosts = GetBuffBoosts("/buff_types/water_resistance");
  for (const auto& boost : water_resistance_boosts) {
    combat_details_.total_water_resistance += boost.flat_boost();
    combat_details_.total_water_resistance += base_water_resistance * boost.ratio_boost();
  }
  
  double base_nature_resistance = 0.1 * (combat_details_.defense_level + combat_details_.magic_level) +
    combat_details_.combat_stats.nature_resistance;
  combat_details_.total_nature_resistance = base_nature_resistance;
  std::vector<Buff> nature_resistance_boosts = GetBuffBoosts("/buff_types/nature_resistance");
  for (const auto& boost : nature_resistance_boosts) {
    combat_details_.total_nature_resistance += boost.flat_boost();
    combat_details_.total_nature_resistance += base_nature_resistance * boost.ratio_boost();
  }
  
  double base_fire_resistance = 0.1 * (combat_details_.defense_level + combat_details_.magic_level) +
    combat_details_.combat_stats.fire_resistance;
  combat_details_.total_fire_resistance = base_fire_resistance;
  std::vector<Buff> fire_resistance_boosts = GetBuffBoosts("/buff_types/fire_resistance");
  for (const auto& boost : fire_resistance_boosts) {
    combat_details_.total_fire_resistance += boost.flat_boost();
    combat_details_.total_fire_resistance += base_fire_resistance * boost.ratio_boost();
  }
  
  // Calculate HP and MP regeneration
  Buff hp_regen_boost = GetBuffBoost("/buff_types/hp_regen");
  combat_details_.combat_stats.hp_regen_per_10 += 
    combat_details_.combat_stats.hp_regen_per_10 * hp_regen_boost.ratio_boost();
  combat_details_.combat_stats.hp_regen_per_10 += hp_regen_boost.flat_boost();
  
  Buff mp_regen_boost = GetBuffBoost("/buff_types/mp_regen");
  combat_details_.combat_stats.mp_regen_per_10 += 
    combat_details_.combat_stats.mp_regen_per_10 * mp_regen_boost.ratio_boost();
  combat_details_.combat_stats.mp_regen_per_10 += mp_regen_boost.flat_boost();
  
  // Add various other buff effects
  combat_details_.combat_stats.life_steal += GetBuffBoost("/buff_types/life_steal").flat_boost();
  combat_details_.combat_stats.physical_thorns += GetBuffBoost("/buff_types/physical_thorns").flat_boost();
  combat_details_.combat_stats.elemental_thorns += GetBuffBoost("/buff_types/elemental_thorns").flat_boost();
  combat_details_.combat_stats.combat_experience += GetBuffBoost("/buff_types/wisdom").flat_boost();
  combat_details_.combat_stats.critical_rate += GetBuffBoost("/buff_types/critical_rate").flat_boost();
  combat_details_.combat_stats.critical_damage += GetBuffBoost("/buff_types/critical_damage").flat_boost();
  combat_details_.combat_stats.cast_speed += GetBuffBoost("/buff_types/cast_speed").flat_boost();
  
  // Calculate drop rates
  Buff combat_drop_rate_boost = GetBuffBoost("/buff_types/combat_drop_rate");
  combat_details_.combat_stats.combat_drop_rate += 
    (1 + combat_details_.combat_stats.combat_drop_rate) * combat_drop_rate_boost.ratio_boost();
  combat_details_.combat_stats.combat_drop_rate += combat_drop_rate_boost.flat_boost();
  
  Buff combat_rare_find_boost = GetBuffBoost("/buff_types/rare_find");
  combat_details_.combat_stats.combat_rare_find += 
    (1 + combat_details_.combat_stats.combat_rare_find) * combat_rare_find_boost.ratio_boost();
  combat_details_.combat_stats.combat_rare_find += combat_rare_find_boost.flat_boost();
  
  // Calculate threat
  double base_threat = 100 + combat_details_.combat_stats.threat;
  combat_details_.total_threat = base_threat;
  Buff threat_boost = GetBuffBoost("/buff_types/threat");
  if (threat_boost.ratio_boost() != 0) {
    combat_details_.combat_stats.threat += base_threat * threat_boost.ratio_boost();
  } else {
    combat_details_.combat_stats.threat = base_threat;
  }
  combat_details_.combat_stats.threat += threat_boost.flat_boost();
}

void CombatUnit::AddCurse(double curse) {
  if (curse_value_ >= 0.1) {
    return;
  }
  
  curse_value_ += curse;
  UpdateCombatDetails();
}

double CombatUnit::UpdateFury(bool is_hit, double fury) {
  if (is_hit && fury_value_ < 0.15) {
    fury_value_ += fury;
  }
  if (!is_hit) {
    fury_value_ = std::floor(fury_value_ / fury / 2) * fury;
  }
  
  return fury_value_;
}

void CombatUnit::AddBuff(const Buff& buff, int64_t current_time) {
  Buff new_buff = buff;
  new_buff.set_start_time(current_time);
  combat_buffs_[buff.unique_hrid()] = new_buff;
  
  UpdateCombatDetails();
}

void CombatUnit::RemoveBuff(const Buff& buff) {
  if (combat_buffs_.find(buff.unique_hrid()) == combat_buffs_.end()) {
    return;
  }
  combat_buffs_.erase(buff.unique_hrid());
  
  UpdateCombatDetails();
}

void CombatUnit::AddPermanentBuff(const Buff& buff) {
  if (permanent_buffs_.find(buff.type_hrid()) != permanent_buffs_.end()) {
    permanent_buffs_[buff.type_hrid()].set_flat_boost(permanent_buffs_[buff.type_hrid()].flat_boost() + buff.flat_boost());
    permanent_buffs_[buff.type_hrid()].set_ratio_boost(permanent_buffs_[buff.type_hrid()].ratio_boost() + buff.ratio_boost());
  } else {
    permanent_buffs_[buff.type_hrid()] = buff;
  }
}

void CombatUnit::GeneratePermanentBuffs() {
  for (const auto& house_room : house_rooms_) {
    for (const auto& buff : house_room.GetBuffs()) {
      AddPermanentBuff(buff);
    }
  }
  
  for (const auto& buff : zone_buffs_) {
    AddPermanentBuff(buff);
  }
}

void CombatUnit::RemoveExpiredBuffs(int64_t current_time) {
  std::vector<std::string> expired_buff_hrids;
  
  for (const auto& [hrid, buff] : combat_buffs_) {
    if (buff.start_time() + buff.duration() <= current_time) {
      expired_buff_hrids.push_back(hrid);
    }
  }
  
  for (const auto& hrid : expired_buff_hrids) {
    combat_buffs_.erase(hrid);
  }
  
  if (!expired_buff_hrids.empty()) {
    UpdateCombatDetails();
  }
}

void CombatUnit::ClearBuffs() {
  combat_buffs_ = permanent_buffs_;
  UpdateCombatDetails();
}

void CombatUnit::ClearCCs() {
  is_stunned_ = false;
  stun_expire_time_ = 0;
  is_silenced_ = false;
  silence_expire_time_ = 0;
  is_blinded_ = false;
  blind_expire_time_ = 0;
  combat_details_.combat_stats.damage_taken = 0.0;
  curse_value_ = 0.0;  // max 0.1
  fury_value_ = 0.0;   // max 0.15
}

std::vector<Buff> CombatUnit::GetBuffBoosts(const std::string& type) {
  std::vector<Buff> boosts;
  
  for (const auto& [hrid, buff] : combat_buffs_) {
    if (buff.type_hrid() == type) {
      Buff boost;
      boost.set_ratio_boost(buff.ratio_boost());
      boost.set_flat_boost(buff.flat_boost());
      boosts.push_back(boost);
    }
  }
  
  return boosts;
}

Buff CombatUnit::GetBuffBoost(const std::string& type) {
  std::vector<Buff> boosts = GetBuffBoosts(type);
  
  Buff boost;
  boost.set_ratio_boost(0.0);
  boost.set_flat_boost(0.0);
  
  for (const auto& b : boosts) {
    boost.set_ratio_boost(boost.ratio_boost() + b.ratio_boost());
    boost.set_flat_boost(boost.flat_boost() + b.flat_boost());
  }
  
  return boost;
}

void CombatUnit::Reset(int64_t current_time) {
  ClearCCs();
  ClearBuffs();
  UpdateCombatDetails();
  ResetCooldowns(current_time);
  
  combat_details_.current_hitpoints = combat_details_.max_hitpoints;
  combat_details_.current_manapoints = combat_details_.max_manapoints;
}

void CombatUnit::ResetCooldowns(int64_t current_time) {
  // Reset food cooldowns
  for (auto& food_item : food_) {
    if (food_item) {
      food_item->SetLastUsed(std::numeric_limits<int64_t>::min());
    }
  }
  
  // Reset drink cooldowns
  for (auto& drink_item : drinks_) {
    if (drink_item) {
      drink_item->SetLastUsed(std::numeric_limits<int64_t>::min());
    }
  }
  
  // Reset ability cooldowns
  double haste = combat_details_.combat_stats.ability_haste;
  
  for (auto& ability : abilities_) {
    if (!ability) continue;
    
    if (is_player_) {
      ability->SetLastUsed(std::numeric_limits<int64_t>::min());
    } else {
      int64_t cooldown_duration = ability->GetCooldownDuration();
      if (haste > 0) {
        cooldown_duration = cooldown_duration * 100 / (100 + haste);
      }
      ability->SetLastUsed(current_time - 
                          static_cast<int64_t>(cooldown_duration * 0.5) + 
                          static_cast<int64_t>(std::rand() % static_cast<int>(cooldown_duration * 0.5)));
    }
  }
}

double CombatUnit::AddHitpoints(double hitpoints) {
  double hitpoints_added = 0.0;
  
  if (combat_details_.current_hitpoints >= combat_details_.max_hitpoints) {
    return hitpoints_added;
  }
  
  double new_hitpoints = std::min(combat_details_.current_hitpoints + hitpoints, 
                                 combat_details_.max_hitpoints);
  hitpoints_added = new_hitpoints - combat_details_.current_hitpoints;
  combat_details_.current_hitpoints = new_hitpoints;
  
  return hitpoints_added;
}

double CombatUnit::AddManapoints(double manapoints) {
  double manapoints_added = 0.0;
  
  if (combat_details_.current_manapoints >= combat_details_.max_manapoints) {
    return manapoints_added;
  }
  
  double new_manapoints = std::min(combat_details_.current_manapoints + manapoints,
                                  combat_details_.max_manapoints);
  manapoints_added = new_manapoints - combat_details_.current_manapoints;
  combat_details_.current_manapoints = new_manapoints;
  
  return manapoints_added;
}

}  // namespace combat_simulator
