// Copyright 2025 BKNMWICombatSimulator
//
// sim_result.h - Simulation result data structure for combat simulator.

#ifndef COMBAT_SIMULATOR_OVERHAUL_INCLUDE_SIM_RESULT_H_
#define COMBAT_SIMULATOR_OVERHAUL_INCLUDE_SIM_RESULT_H_

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace combat_simulator {

struct TimeAliveEntry {
  std::string name;
  double time_spent_alive = 0;
  double spawned_at = 0;
  bool alive = true;
  int count = 0;
};

class SimResult {
 public:
  SimResult(const std::string& zone_name, int number_of_players, const std::string& sim_id = "");

  void AddDeath(const std::string& hrid);
  void UpdateTimeSpentAlive(const std::string& name, bool alive, double time);
  void AddExperienceGain(const std::string& hrid, const std::string& type, double experience, double exp_multiplier = 1.0);
  void AddEncounterEnd();
  void AddAttack(const std::string& source, const std::string& target, const std::string& ability, const std::string& hit);
  void AddConsumableUse(const std::string& hrid, const std::string& consumable_hrid);
  void AddHitpointsGained(const std::string& hrid, const std::string& source, double amount);
  void AddManapointsGained(const std::string& hrid, const std::string& source, double amount);
  void SetDropRateMultiplier(const std::string& hrid, double multiplier);
  void SetRareFindMultiplier(const std::string& hrid, double multiplier);
  void SetManaUsed(const std::string& hrid, const std::unordered_map<std::string, double>& mana_used);
  void AddHitpointsSpent(const std::string& hrid, const std::string& source, double amount);
  void AddRanOutOfManaCount(const std::string& hrid, bool is_run_out_of_mana);

  // 公共成员变量（可根据需要改为私有+访问器）
  std::string sim_id_;
  std::string simulation_name_;
  std::string zone_name_;
  int number_of_players_;
  double simulated_time_ = 0;
  bool is_dungeon_ = false;
  int dungeons_completed_ = 0;
  int dungeons_failed_ = 0;
  int max_wave_reached_ = 0;
  int encounters_ = 0;
  int elite_tier_ = 0;
  std::vector<std::string> boss_spawns_;
  std::vector<TimeAliveEntry> time_spent_alive_;
  std::unordered_map<std::string, int> deaths_;
  std::unordered_map<std::string, std::unordered_map<std::string, double>> drop_rate_multiplier_;
  std::unordered_map<std::string, std::unordered_map<std::string, double>> rare_find_multiplier_;
  std::unordered_map<std::string, std::unordered_map<std::string, double>> mana_used_;
  std::unordered_map<std::string, std::vector<int>> player_ran_out_of_mana_time_;
  std::unordered_map<std::string, bool> player_ran_out_of_mana_;
  std::unordered_map<std::string, std::unordered_map<std::string, double>> hitpoints_spent_;
  std::unordered_map<std::string, std::unordered_map<std::string, double>> hitpoints_gained_;
  std::unordered_map<std::string, std::unordered_map<std::string, double>> manapoints_gained_;
  std::unordered_map<std::string, std::unordered_map<std::string, int>> consumables_used_;
  std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, int>>>> attacks_;
  std::unordered_map<std::string, std::unordered_map<std::string, double>> experience_gained_;
};

}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_OVERHAUL_INCLUDE_SIM_RESULT_H_
