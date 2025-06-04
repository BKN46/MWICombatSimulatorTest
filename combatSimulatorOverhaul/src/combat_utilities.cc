// combat_utilities.cc - Utility function implementations for combat simulation.
// Copyright 2025 BKNMWICombatSimulator

#include <algorithm>
#include <cmath>
#include <random>
#include <string>

#include "combat_utilities.h"
#include "combat_unit.h"
#include "ability.h"

namespace combat_simulator {

std::shared_ptr<CombatUnit> CombatUtilities::GetTarget(const std::vector<std::shared_ptr<CombatUnit>>& enemies) {
  for (const auto& enemy : enemies) {
    if (enemy && enemy->combat_details().current_hitpoints > 0) {
      return enemy;
    }
  }
  return nullptr;
}

int CombatUtilities::RandomInt(double min, double max) {
  if (max < min) std::swap(min, max);
  int min_ceil = static_cast<int>(std::ceil(min));
  int max_floor = static_cast<int>(std::floor(max));
  static thread_local std::mt19937 gen(std::random_device{}());
  if (std::floor(min) == max_floor) {
    return static_cast<int>(std::floor((min + max) / 2 + std::uniform_real_distribution<>(0, 1)(gen)));
  }
  double min_tail = -1 * (min - min_ceil);
  double max_tail = max - max_floor;
  double balanced_weight = 2 * min_tail + (max_floor - min_ceil);
  double balanced_average = (max_floor + min_ceil) / 2.0;
  double average = (max + min) / 2.0;
  double extra_tail_weight = (balanced_weight * (average - balanced_average)) / (max_floor + 1 - average);
  double extra_tail_chance = std::abs(extra_tail_weight / (extra_tail_weight + balanced_weight));
  double rand_val = std::uniform_real_distribution<>(0, 1)(gen);
  if (rand_val < extra_tail_chance) {
    if (max_tail > min_tail) {
      return static_cast<int>(std::floor(max_floor + 1));
    } else {
      return static_cast<int>(std::floor(min_ceil - 1));
    }
  }
  if (max_tail > min_tail) {
    return static_cast<int>(std::floor(min + std::uniform_real_distribution<>(0, 1)(gen) * (max_floor + min_tail - min + 1)));
  } else {
    return static_cast<int>(std::floor(min_ceil - max_tail + std::uniform_real_distribution<>(0, 1)(gen) * (max - (min_ceil - max_tail) + 1)));
  }
}

int CombatUtilities::CalculateTickValue(int total_value, int total_ticks, int current_tick) {
  int current_sum = static_cast<int>(std::floor((static_cast<double>(current_tick) * total_value) / total_ticks));
  int previous_sum = static_cast<int>(std::floor((static_cast<double>(current_tick - 1) * total_value) / total_ticks));
  return current_sum - previous_sum;
}

double CombatUtilities::CalculateStaminaExperience(double damage_prevented, double damage_taken) {
  return 0.03 * damage_prevented + 0.3 * damage_taken;
}

double CombatUtilities::CalculateIntelligenceExperience(double mana_used) {
  return 0.3 * mana_used;
}

double CombatUtilities::CalculateAttackExperience(double damage, double damage_prevented, const std::string& combat_style) {
  if (combat_style == "/combat_styles/stab") {
    return 0.54 + 0.1125 * (damage + 0.35 * damage_prevented);
  } else if (combat_style == "/combat_styles/slash") {
    return 0.3 + 0.0625 * (damage + 0.35 * damage_prevented);
  } else if (combat_style == "/combat_styles/smash") {
    return 0.06 + 0.0125 * (damage + 0.35 * damage_prevented);
  }
  return 0.0;
}

double CombatUtilities::CalculatePowerExperience(double damage, double damage_prevented, const std::string& combat_style) {
  if (combat_style == "/combat_styles/stab") {
    return 0.06 + 0.0125 * (damage + 0.35 * damage_prevented);
  } else if (combat_style == "/combat_styles/slash") {
    return 0.3 + 0.0625 * (damage + 0.35 * damage_prevented);
  } else if (combat_style == "/combat_styles/smash") {
    return 0.54 + 0.1125 * (damage + 0.35 * damage_prevented);
  }
  return 0.0;
}

double CombatUtilities::CalculateDefenseExperience(double damage_prevented) {
  return 0.4 + 0.1 * damage_prevented;
}

double CombatUtilities::CalculateRangedExperience(double damage, double damage_prevented) {
  return 0.4 + 0.083375 * (damage + 0.35 * damage_prevented);
}

double CombatUtilities::CalculateMagicExperience(double damage, double damage_prevented) {
  return 0.4 + 0.083375 * (damage + 0.35 * damage_prevented);
}

double CombatUtilities::CalculateHealingExperience(double healed) {
  return CalculateMagicExperience(healed, 0) * 3;
}

CombatUtilities::AttackResult CombatUtilities::ProcessAttack(
    CombatUnit& source, CombatUnit& target, const AbilityEffect* ability_effect) {
  CombatUtilities::AttackResult result;
  // 1. 确定战斗风格和伤害类型
  std::string combat_style = ability_effect ? ability_effect->combat_style_hrid : source.combat_details().combat_stats.combat_style_hrid;
  std::string damage_type = ability_effect ? ability_effect->damage_type : source.combat_details().combat_stats.damage_type;

  double source_accuracy_rating = 1;
  double source_auto_attack_max_damage = 1;
  double target_evasion_rating = 1;

  if (combat_style == "/combat_styles/stab") {
    source_accuracy_rating = source.combat_details().stab_accuracy_rating;
    source_auto_attack_max_damage = source.combat_details().stab_max_damage;
    target_evasion_rating = target.combat_details().stab_evasion_rating;
  } else if (combat_style == "/combat_styles/slash") {
    source_accuracy_rating = source.combat_details().slash_accuracy_rating;
    source_auto_attack_max_damage = source.combat_details().slash_max_damage;
    target_evasion_rating = target.combat_details().slash_evasion_rating;
  } else if (combat_style == "/combat_styles/smash") {
    source_accuracy_rating = source.combat_details().smash_accuracy_rating;
    source_auto_attack_max_damage = source.combat_details().smash_max_damage;
    target_evasion_rating = target.combat_details().smash_evasion_rating;
  } else if (combat_style == "/combat_styles/ranged") {
    source_accuracy_rating = source.combat_details().ranged_accuracy_rating;
    source_auto_attack_max_damage = source.combat_details().ranged_max_damage;
    target_evasion_rating = target.combat_details().ranged_evasion_rating;
  } else if (combat_style == "/combat_styles/magic") {
    source_accuracy_rating = source.combat_details().magic_accuracy_rating;
    source_auto_attack_max_damage = source.combat_details().magic_max_damage;
    target_evasion_rating = target.combat_details().magic_evasion_rating;
  }

  double source_damage_multiplier = 1;
  double source_resistance = 0;
  double source_penetration = 0;
  double target_resistance = 0;
  double target_thorn_power = 0;
  double target_penetration = 0;
  std::string thorn_type;

  if (damage_type == "/damage_types/physical") {
    source_damage_multiplier = 1 + source.combat_details().combat_stats.physical_amplify;
    source_resistance = source.combat_details().total_armor;
    source_penetration = source.combat_details().combat_stats.armor_penetration;
    target_resistance = target.combat_details().total_armor;
    target_thorn_power = target.combat_details().combat_stats.physical_thorns;
    target_penetration = target.combat_details().combat_stats.armor_penetration;
    thorn_type = "physicalThorns";
  } else if (damage_type == "/damage_types/water") {
    source_damage_multiplier = 1 + source.combat_details().combat_stats.water_amplify;
    source_resistance = source.combat_details().total_water_resistance;
    source_penetration = source.combat_details().combat_stats.water_penetration;
    target_resistance = target.combat_details().total_water_resistance;
    target_thorn_power = target.combat_details().combat_stats.elemental_thorns;
    thorn_type = "elementalThorns";
  } else if (damage_type == "/damage_types/nature") {
    source_damage_multiplier = 1 + source.combat_details().combat_stats.nature_amplify;
    source_resistance = source.combat_details().total_nature_resistance;
    source_penetration = source.combat_details().combat_stats.nature_penetration;
    target_resistance = target.combat_details().total_nature_resistance;
    target_thorn_power = target.combat_details().combat_stats.elemental_thorns;
    thorn_type = "elementalThorns";
  } else if (damage_type == "/damage_types/fire") {
    source_damage_multiplier = 1 + source.combat_details().combat_stats.fire_amplify;
    source_resistance = source.combat_details().total_fire_resistance;
    source_penetration = source.combat_details().combat_stats.fire_penetration;
    target_resistance = target.combat_details().total_fire_resistance;
    target_thorn_power = target.combat_details().combat_stats.elemental_thorns;
    thorn_type = "elementalThorns";
  }

  double hit_chance = 1;
  double crit_chance = 0;
  double bonus_crit_chance = source.combat_details().combat_stats.critical_rate;
  double bonus_crit_damage = source.combat_details().combat_stats.critical_damage;

  if (ability_effect) {
    source_accuracy_rating *= (1 + ability_effect->bonus_accuracy_ratio);
  }
  if (source.is_weakened()) {
    source_accuracy_rating = source_accuracy_rating - (source.weaken_percentage() * source_accuracy_rating);
  }
  hit_chance = std::pow(source_accuracy_rating, 1.4) /
      (std::pow(source_accuracy_rating, 1.4) + std::pow(target_evasion_rating, 1.4));
  if (combat_style == "/combat_styles/ranged") {
    crit_chance = 0.3 * hit_chance;
  }
  crit_chance = crit_chance + bonus_crit_chance;

  double base_damage_flat = ability_effect ? ability_effect->damage_flat : 0;
  double base_damage_ratio = ability_effect ? ability_effect->damage_ratio : 1;
  double armor_damage_ratio_flat = ability_effect ? ability_effect->armor_damage_ratio * source.combat_details().total_armor : 0;

  double source_min_damage = source_damage_multiplier * (1 + base_damage_flat + armor_damage_ratio_flat);
  double source_max_damage = source_damage_multiplier * (base_damage_ratio * source_auto_attack_max_damage + base_damage_flat + armor_damage_ratio_flat);

  if (CombatUtilities::RandomInt(0, 10000) < static_cast<int>(crit_chance * 10000)) {
    source_max_damage = source_max_damage * (1 + bonus_crit_damage);
    source_min_damage = source_max_damage;
  }

  int damage_roll = CombatUtilities::RandomInt(source_min_damage, source_max_damage);
  damage_roll = static_cast<int>(damage_roll * (1 + source.combat_details().combat_stats.task_damage));
  damage_roll = static_cast<int>(damage_roll * (1 + target.combat_details().combat_stats.damage_taken));
  if (!ability_effect) {
    damage_roll += static_cast<int>(damage_roll * source.combat_details().combat_stats.auto_attack_damage);
  }
  int max_premitigated_damage = std::min(damage_roll, static_cast<int>(target.combat_details().current_hitpoints));

  int damage_done = 0;
  int reflect_damage = 0;
  int mitigated_reflect_damage = 0;
  int reflect_damage_done = 0;
  bool did_hit = false;
  if (CombatUtilities::RandomInt(0, 10000) < static_cast<int>(hit_chance * 10000)) {
    did_hit = true;
    double penetrated_target_resistance = target_resistance;
    if (source_penetration > 0 && target_resistance > 0) {
      penetrated_target_resistance = target_resistance / (1 + source_penetration);
    }
    double target_damage_taken_ratio = 100.0 / (100.0 + penetrated_target_resistance);
    if (penetrated_target_resistance < 0) {
      target_damage_taken_ratio = (100.0 - penetrated_target_resistance) / 100.0;
    }
    int mitigated_damage = static_cast<int>(std::ceil(target_damage_taken_ratio * damage_roll));
    damage_done = std::min(mitigated_damage, static_cast<int>(target.combat_details().current_hitpoints));
    target.AddHitpoints(-damage_done);
  }
  if (target_thorn_power > 0 && target_resistance > 0) {
    double penetrated_source_resistance = source_resistance;
    if (target_penetration > 0 && source_resistance > 0) {
      penetrated_source_resistance = source_resistance / (1 + target_penetration);
    }
    double source_damage_taken_ratio = 100.0 / (100.0 + penetrated_source_resistance);
    if (penetrated_source_resistance < 0) {
      source_damage_taken_ratio = (100.0 - penetrated_source_resistance) / 100.0;
    }
    reflect_damage = static_cast<int>(std::ceil(target_thorn_power * target_resistance));
    mitigated_reflect_damage = static_cast<int>(std::ceil(source_damage_taken_ratio * reflect_damage));
    reflect_damage_done = std::min(mitigated_reflect_damage, static_cast<int>(source.combat_details().current_hitpoints));
    source.AddHitpoints(-reflect_damage_done);
  }
  int life_steal_heal = 0;
  if (!ability_effect && did_hit && source.combat_details().combat_stats.life_steal > 0) {
    life_steal_heal = static_cast<int>(source.AddHitpoints(std::floor(source.combat_details().combat_stats.life_steal * damage_done)));
  }
  int hp_drain = 0;
  if (ability_effect && did_hit && ability_effect->hp_drain_ratio > 0) {
    double healing_amplify = 1 + source.combat_details().combat_stats.healing_amplify;
    hp_drain = static_cast<int>(source.AddHitpoints(std::floor(ability_effect->hp_drain_ratio * damage_done * healing_amplify)));
  }
  int mana_leech_mana = 0;
  if (!ability_effect && did_hit && source.combat_details().combat_stats.mana_leech > 0) {
    mana_leech_mana = static_cast<int>(source.AddManapoints(std::floor(source.combat_details().combat_stats.mana_leech * damage_done)));
  }
  // 经验相关
  if (did_hit) {
    double damage_prevented = max_premitigated_damage - damage_done;
    if (damage_prevented < 0) damage_prevented = 0;
    if (combat_style == "/combat_styles/stab" || combat_style == "/combat_styles/slash" || combat_style == "/combat_styles/smash") {
      result.experience_gained_source["attack"] = CalculateAttackExperience(damage_done, damage_prevented, combat_style);
      result.experience_gained_source["power"] = CalculatePowerExperience(damage_done, damage_prevented, combat_style);
    } else if (combat_style == "/combat_styles/ranged") {
      result.experience_gained_source["ranged"] = CalculateRangedExperience(damage_done, damage_prevented);
    } else if (combat_style == "/combat_styles/magic") {
      result.experience_gained_source["magic"] = CalculateMagicExperience(damage_done, damage_prevented);
    }
    result.experience_gained_target["defense"] = CalculateDefenseExperience(damage_prevented);
    result.experience_gained_target["stamina"] = CalculateStaminaExperience(damage_prevented, damage_done);
    // Reflect damage experience
    if (reflect_damage_done > 0) {
      double reflect_damage_prevented = reflect_damage - reflect_damage_done;
      if (reflect_damage_prevented < 0) reflect_damage_prevented = 0;
      result.experience_gained_target["defense"] += CalculateDefenseExperience(mitigated_reflect_damage);
      result.experience_gained_source["defense"] = CalculateDefenseExperience(reflect_damage_prevented);
      result.experience_gained_source["stamina"] = CalculateStaminaExperience(reflect_damage_prevented, reflect_damage_done);
    }
  }
  return result;
}

int CombatUtilities::ProcessHeal(CombatUnit& source, const AbilityEffect& ability_effect, CombatUnit& target) {
  // 只支持magic风格
  if (ability_effect.combat_style_hrid != "/combat_styles/magic") {
    return 0;
  }
  double healing_amplify = 1 + source.combat_details().combat_stats.healing_amplify;
  double magic_max_damage = source.combat_details().magic_max_damage;
  double base_heal_flat = ability_effect.damage_flat;
  double base_heal_ratio = ability_effect.damage_ratio;
  double min_heal = healing_amplify * (1 + base_heal_flat);
  double max_heal = healing_amplify * (base_heal_ratio * magic_max_damage + base_heal_flat);
  int heal = CombatUtilities::RandomInt(min_heal, max_heal);
  int amount_healed = static_cast<int>(target.AddHitpoints(heal));
  return amount_healed;
}

int CombatUtilities::ProcessRevive(CombatUnit& source, const AbilityEffect& ability_effect, CombatUnit& target) {
  // 只支持magic风格
  if (ability_effect.combat_style_hrid != "/combat_styles/magic") {
    return 0;
  }
  double healing_amplify = 1 + source.combat_details().combat_stats.healing_amplify;
  double magic_max_damage = source.combat_details().magic_max_damage;
  double base_heal_flat = ability_effect.damage_flat;
  double base_heal_ratio = ability_effect.damage_ratio;
  double min_heal = healing_amplify * (1 + base_heal_flat);
  double max_heal = healing_amplify * (base_heal_ratio * magic_max_damage + base_heal_flat);
  int heal = CombatUtilities::RandomInt(min_heal, max_heal);
  int amount_healed = static_cast<int>(target.AddHitpoints(heal));
  // 恢复MP到最大值
  double mp_to_full = target.combat_details().max_manapoints - target.combat_details().current_manapoints;
  target.AddManapoints(mp_to_full);
  target.ClearCCs();
  target.ClearBuffs();
  return amount_healed;
}

int CombatUtilities::ProcessSpendHp(CombatUnit& source, const AbilityEffect& ability_effect) {
  double current_hp = source.combat_details().current_hitpoints;
  double spend_hp_ratio = ability_effect.spend_hp_ratio;
  int spent_hp = static_cast<int>(std::floor(current_hp * spend_hp_ratio));
  source.AddHitpoints(-spent_hp); // 用封装方法减少HP
  return spent_hp;
}

}  // namespace combat_simulator
