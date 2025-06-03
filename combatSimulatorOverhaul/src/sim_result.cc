// Copyright 2025 BKNMWICombatSimulator
//
// sim_result.cc - Implementation of SimResult for combat simulator.

#include "sim_result.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <sstream>

namespace combat_simulator {

SimResult::SimResult(const std::string& zone_name, int number_of_players, const std::string& sim_id)
    : sim_id_(sim_id.empty() ? std::to_string(rand()) : sim_id),
      zone_name_(zone_name),
      number_of_players_(number_of_players) {
  // 生成仿真时间字符串
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  simulation_name_ = std::ctime(&now_c);
  if (!simulation_name_.empty() && simulation_name_.back() == '\n') simulation_name_.pop_back();
}

void SimResult::AddDeath(const std::string& hrid) {
  deaths_[hrid]++;
}

void SimResult::UpdateTimeSpentAlive(const std::string& name, bool alive, double time) {
  auto it = std::find_if(time_spent_alive_.begin(), time_spent_alive_.end(), [&](const TimeAliveEntry& e) { return e.name == name; });
  if (alive) {
    if (it != time_spent_alive_.end()) {
      it->alive = true;
      it->spawned_at = time;
    } else {
      time_spent_alive_.push_back({name, 0, time, true, 0});
    }
  } else if (it != time_spent_alive_.end()) {
    double time_alive = time - it->spawned_at;
    it->alive = false;
    it->time_spent_alive += time_alive;
    it->count += 1;
  }
}

void SimResult::AddExperienceGain(const std::string& hrid, const std::string& type, double experience, double exp_multiplier) {
  experience_gained_[hrid][type] += experience * exp_multiplier;
}

void SimResult::AddEncounterEnd() {
  encounters_++;
}

void SimResult::AddAttack(const std::string& source, const std::string& target, const std::string& ability, const std::string& hit) {
  attacks_[source][target][ability][hit]++;
}

void SimResult::AddConsumableUse(const std::string& hrid, const std::string& consumable_hrid) {
  consumables_used_[hrid][consumable_hrid]++;
}

void SimResult::AddHitpointsGained(const std::string& hrid, const std::string& source, double amount) {
  hitpoints_gained_[hrid][source] += amount;
}

void SimResult::AddManapointsGained(const std::string& hrid, const std::string& source, double amount) {
  manapoints_gained_[hrid][source] += amount;
}

void SimResult::SetDropRateMultiplier(const std::string& hrid, double multiplier) {
  drop_rate_multiplier_[hrid]["multiplier"] = multiplier;
}

void SimResult::SetRareFindMultiplier(const std::string& hrid, double multiplier) {
  rare_find_multiplier_[hrid]["multiplier"] = multiplier;
}

void SimResult::SetManaUsed(const std::string& hrid, const std::unordered_map<std::string, double>& mana_used) {
  mana_used_[hrid] = mana_used;
}

void SimResult::AddHitpointsSpent(const std::string& hrid, const std::string& source, double amount) {
  hitpoints_spent_[hrid][source] += amount;
}

void SimResult::AddRanOutOfManaCount(const std::string& hrid, bool is_run_out_of_mana) {
  auto& arr = player_ran_out_of_mana_time_[hrid];
  if (arr.empty()) arr = {0, 0};
  if (is_run_out_of_mana) {
    player_ran_out_of_mana_[hrid] = true;
    arr[0] += 1;
  } else {
    arr[1] += 1;
  }
}

}  // namespace combat_simulator
