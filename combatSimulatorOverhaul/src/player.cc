// Copyright 2025 BKNMWICombatSimulator
//
// player.cc - Implementation of the Player class for the combat simulator.

#include "player.h"

#include <algorithm>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ability.h"
#include "combat_unit.h"
#include "consumable.h"
#include "equipment.h"
#include "house_room.h"
#include "nlohmann/json.hpp"

namespace combat_simulator {

Player::Player() : CombatUnit() {
  is_player_ = true;
  hrid_ = "player";
  // Initialize equipment slots
  equipment_ = {
      {"/equipment_types/head", nullptr},
      {"/equipment_types/body", nullptr},
      {"/equipment_types/legs", nullptr},
      {"/equipment_types/feet", nullptr},
      {"/equipment_types/hands", nullptr},
      {"/equipment_types/main_hand", nullptr},
      {"/equipment_types/two_hand", nullptr},
      {"/equipment_types/off_hand", nullptr},
      {"/equipment_types/pouch", nullptr},
      {"/equipment_types/back", nullptr},
  };
}

std::unique_ptr<Player> Player::CreateFromDTO(const nlohmann::json& dto) {
  auto player = std::make_unique<Player>();
  // Set basic stats
  player->stamina_level_ = dto.value("staminaLevel", 0);
  player->intelligence_level_ = dto.value("intelligenceLevel", 0);
  player->attack_level_ = dto.value("attackLevel", 0);
  player->power_level_ = dto.value("powerLevel", 0);
  player->defense_level_ = dto.value("defenseLevel", 0);
  player->ranged_level_ = dto.value("rangedLevel", 0);
  player->magic_level_ = dto.value("magicLevel", 0);
  player->hrid_ = dto.value("hrid", "player");

  // Equipment
  if (dto.contains("equipment")) {
    for (const auto& [key, value] : dto["equipment"].items()) {
      if (!value.is_null()) {
        player->equipment_[key] = Equipment::CreateFromDTO(value);
      } else {
        player->equipment_[key] = nullptr;
      }
    }
  }

  // Food
  if (dto.contains("food")) {
    for (const auto& food : dto["food"]) {
      if (!food.is_null()) {
        player->food_.push_back(Consumable::CreateFromDTO(food));
      } else {
        player->food_.push_back(nullptr);
      }
    }
  }

  // Drinks
  if (dto.contains("drinks")) {
    for (const auto& drink : dto["drinks"]) {
      if (!drink.is_null()) {
        player->drinks_.push_back(Consumable::CreateFromDTO(drink));
      } else {
        player->drinks_.push_back(nullptr);
      }
    }
  }

  // Abilities
  if (dto.contains("abilities")) {
    for (const auto& ability : dto["abilities"]) {
      if (!ability.is_null()) {
        player->abilities_.push_back(Ability::CreateFromDTO(ability));
      } else {
        player->abilities_.push_back(nullptr);
      }
    }
  }

  // House rooms
  if (dto.contains("houseRooms")) {
    for (const auto& [room, level] : dto["houseRooms"].items()) {
      if (level.is_number() && level.get<int>() > 0) {
        player->house_rooms_.push_back(std::make_shared<HouseRoom>(room, level.get<int>()));
      }
    }
  }

  return player;
}

void Player::UpdateCombatDetails() {
  // Main hand or two hand
  auto main_hand = equipment_["/equipment_types/main_hand"];
  auto two_hand = equipment_["/equipment_types/two_hand"];
  if (main_hand) {
    combat_details_.combat_stats.combat_style_hrid = main_hand->GetCombatStyle();
    combat_details_.combat_stats.damage_type = main_hand->GetDamageType();
    combat_details_.combat_stats.attack_interval = main_hand->GetCombatStat("attackInterval");
  } else if (two_hand) {
    combat_details_.combat_stats.combat_style_hrid = two_hand->GetCombatStyle();
    combat_details_.combat_stats.damage_type = two_hand->GetDamageType();
    combat_details_.combat_stats.attack_interval = two_hand->GetCombatStat("attackInterval");
  } else {
    combat_details_.combat_stats.combat_style_hrid = "/combat_styles/smash";
    combat_details_.combat_stats.damage_type = "/damage_types/physical";
    combat_details_.combat_stats.attack_interval = 3000000000;
  }

  // Stat list
  const std::vector<std::string> stat_list = {
      "stabAccuracy", "slashAccuracy", "smashAccuracy", "rangedAccuracy", "magicAccuracy",
      "stabDamage", "slashDamage", "smashDamage", "rangedDamage", "magicDamage", "taskDamage",
      "physicalAmplify", "waterAmplify", "natureAmplify", "fireAmplify", "healingAmplify",
      "stabEvasion", "slashEvasion", "smashEvasion", "rangedEvasion", "magicEvasion", "armor",
      "waterResistance", "natureResistance", "fireResistance", "maxHitpoints", "maxManapoints",
      "lifeSteal", "hpRegenPer10", "mpRegenPer10", "physicalThorns", "elementalThorns",
      "combatDropRate", "combatRareFind", "combatDropQuantity", "combatExperience", "criticalRate",
      "criticalDamage", "armorPenetration", "waterPenetration", "naturePenetration", "firePenetration",
      "abilityHaste", "tenacity", "manaLeech", "castSpeed", "threat", "parry", "mayhem", "pierce",
      "curse", "fury", "weaken", "ripple", "bloom", "blaze", "attackSpeed", "foodHaste",
      "drinkConcentration", "autoAttackDamage"
  };

  for (const auto& stat : stat_list) {
    int stat_sum = 0;
    for (const auto& [_, equip] : equipment_) {
      if (equip) {
        stat_sum += equip->GetCombatStat(stat);
      }
    }
    combat_details_.combat_stats.SetStat(stat, stat_sum);
  }

  // Food/drink slots
  auto pouch = equipment_["/equipment_types/pouch"];
  if (pouch) {
    combat_details_.combat_stats.food_slots = 1 + pouch->GetCombatStat("foodSlots");
    combat_details_.combat_stats.drink_slots = 1 + pouch->GetCombatStat("drinkSlots");
  } else {
    combat_details_.combat_stats.food_slots = 1;
    combat_details_.combat_stats.drink_slots = 1;
  }

  // Call base class update
  CombatUnit::UpdateCombatDetails();
}

}  // namespace combat_simulator
