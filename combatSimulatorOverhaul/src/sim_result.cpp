#include "combat_simulator_overhaul/sim_result.h"

#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>

namespace combat_simulator_overhaul {

// Forward declarations for classes referenced but not defined
class Unit;
class Consumable;

SimResult::SimResult(const std::string& zone_name, int number_of_players, const std::string& sim_id) 
    : zone_name_(zone_name),
      number_of_players_(number_of_players),
      encounters_(0),
      elite_tier_(0),
      is_dungeon_(false),
      dungeons_completed_(0),
      dungeons_failed_(0),
      max_wave_reached_(0) {
  
  // Generate a random ID if not provided
  if (sim_id.empty()) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 999999999);
    sim_id_ = std::to_string(dis(gen));
  } else {
    sim_id_ = sim_id;
  }
  
  // Set simulation time to current time
  simulation_time_ = std::time(nullptr);
  
  // Format simulation name
  std::tm tm = *std::localtime(&simulation_time_);
  std::stringstream ss;
  ss << std::put_time(&tm, "%c");
  simulation_name_ = ss.str();
  
  // Initialize player out of mana status
  player_ran_out_of_mana_["player1"] = false;
  player_ran_out_of_mana_["player2"] = false;
  player_ran_out_of_mana_["player3"] = false;
  player_ran_out_of_mana_["player4"] = false;
  player_ran_out_of_mana_["player5"] = false;
}

void SimResult::AddDeath(const Unit* unit) {
  if (deaths_.find(unit->hrid) == deaths_.end()) {
    deaths_[unit->hrid] = 0;
  }
  deaths_[unit->hrid] += 1;
}

void SimResult::UpdateTimeSpentAlive(const std::string& name, bool alive, double time) {
  auto it = std::find_if(time_spent_alive_.begin(), time_spent_alive_.end(), 
                        [&name](const TimeAliveInfo& info) { return info.name == name; });
                        
  if (alive) {
    if (it != time_spent_alive_.end()) {
      it->alive = true;
      it->spawned_at = time;
    } else {
      time_spent_alive_.push_back({name, 0, time, true, 0});
    }
  } else {
    if (it != time_spent_alive_.end()) {
      double time_alive = time - it->spawned_at;
      it->alive = false;
      it->time_spent_alive += time_alive;
      it->count += 1;
    }
  }
}

void SimResult::AddExperienceGain(const Unit* unit, const std::string& type, double experience) {
  if (!unit->isPlayer) {
    return;
  }

  if (experience_gained_.find(unit->hrid) == experience_gained_.end()) {
    experience_gained_[unit->hrid] = {
      {"stamina", 0},
      {"intelligence", 0},
      {"attack", 0},
      {"power", 0},
      {"defense", 0},
      {"ranged", 0},
      {"magic", 0}
    };
  }

  experience_gained_[unit->hrid][type] += experience * (1 + unit->combatDetails.combatStats.combatExperience);
}

void SimResult::AddEncounterEnd() {
  encounters_++;
}

void SimResult::AddAttack(const Unit* source, const Unit* target, const std::string& ability, const std::string& hit) {
  if (attacks_[source->hrid][target->hrid][ability].find(hit) == 
      attacks_[source->hrid][target->hrid][ability].end()) {
    attacks_[source->hrid][target->hrid][ability][hit] = 0;
  }

  attacks_[source->hrid][target->hrid][ability][hit] += 1;
}

void SimResult::AddConsumableUse(const Unit* unit, const Consumable* consumable) {
  if (consumables_used_.find(unit->hrid) == consumables_used_.end()) {
    consumables_used_[unit->hrid] = {};
  }
  
  if (consumables_used_[unit->hrid].find(consumable->hrid) == consumables_used_[unit->hrid].end()) {
    consumables_used_[unit->hrid][consumable->hrid] = 0;
  }

  consumables_used_[unit->hrid][consumable->hrid] += 1;
}

void SimResult::AddHitpointsGained(const Unit* unit, const std::string& source, double amount) {
  if (hitpoints_gained_.find(unit->hrid) == hitpoints_gained_.end()) {
    hitpoints_gained_[unit->hrid] = {};
  }
  
  if (hitpoints_gained_[unit->hrid].find(source) == hitpoints_gained_[unit->hrid].end()) {
    hitpoints_gained_[unit->hrid][source] = 0;
  }

  hitpoints_gained_[unit->hrid][source] += amount;
}

void SimResult::AddManapointsGained(const Unit* unit, const std::string& source, double amount) {
  if (manapoints_gained_.find(unit->hrid) == manapoints_gained_.end()) {
    manapoints_gained_[unit->hrid] = {};
  }
  
  if (manapoints_gained_[unit->hrid].find(source) == manapoints_gained_[unit->hrid].end()) {
    manapoints_gained_[unit->hrid][source] = 0;
  }

  manapoints_gained_[unit->hrid][source] += amount;
}

void SimResult::SetDropRateMultipliers(const Unit* unit) {
  drop_rate_multiplier_[unit->hrid] = 1 + unit->combatDetails.combatStats.combatDropRate;
  rare_find_multiplier_[unit->hrid] = 1 + unit->combatDetails.combatStats.combatRareFind;
}

void SimResult::SetManaUsed(const Unit* unit) {
  mana_used_[unit->hrid] = {};
  for (const auto& entry : unit->abilityManaCosts) {
    mana_used_[unit->hrid][entry.first] = entry.second;
  }
}

void SimResult::AddHitpointsSpent(const Unit* unit, const std::string& source, double amount) {
  if (hitpoints_spent_.find(unit->hrid) == hitpoints_spent_.end()) {
    hitpoints_spent_[unit->hrid] = {};
  }
  
  if (hitpoints_spent_[unit->hrid].find(source) == hitpoints_spent_[unit->hrid].end()) {
    hitpoints_spent_[unit->hrid][source] = 0;
  }

  hitpoints_spent_[unit->hrid][source] += amount;
}

void SimResult::AddRanOutOfManaCount(const Unit* unit, bool is_run_out_of_mana) {
  if (player_ran_out_of_mana_time_.find(unit->hrid) == player_ran_out_of_mana_time_.end()) {
    player_ran_out_of_mana_time_[unit->hrid] = {0, 0};
  }
  
  if (is_run_out_of_mana) {
    player_ran_out_of_mana_[unit->hrid] = true;
    player_ran_out_of_mana_time_[unit->hrid][0] += 1;
  } else {
    player_ran_out_of_mana_time_[unit->hrid][1] += 1;
  }
}

}  // namespace combat_simulator_overhaul
