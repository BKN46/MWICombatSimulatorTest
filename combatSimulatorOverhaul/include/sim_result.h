#ifndef COMBAT_SIMULATOR_OVERHAUL_SIM_RESULT_H_
#define COMBAT_SIMULATOR_OVERHAUL_SIM_RESULT_H_

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <random>

namespace combat_simulator_overhaul {

struct TimeAliveInfo {
  std::string name;
  double time_spent_alive;
  double spawned_at;
  bool alive;
  int count;
};

struct BossSpawn {
  std::string name;
  double time;
};

class SimResult {
 public:
  SimResult(const std::string& zone_name, int number_of_players, const std::string& sim_id = "");
  
  void AddDeath(const class Unit* unit);
  void UpdateTimeSpentAlive(const std::string& name, bool alive, double time);
  void AddExperienceGain(const Unit* unit, const std::string& type, double experience);
  void AddEncounterEnd();
  void AddAttack(const Unit* source, const Unit* target, const std::string& ability, const std::string& hit);
  void AddConsumableUse(const Unit* unit, const class Consumable* consumable);
  void AddHitpointsGained(const Unit* unit, const std::string& source, double amount);
  void AddManapointsGained(const Unit* unit, const std::string& source, double amount);
  void SetDropRateMultipliers(const Unit* unit);
  void SetManaUsed(const Unit* unit);
  void AddHitpointsSpent(const Unit* unit, const std::string& source, double amount);
  void AddRanOutOfManaCount(const Unit* unit, bool is_run_out_of_mana);

 private:
  std::string sim_id_;
  std::time_t simulation_time_;
  std::string simulation_name_;
  std::unordered_map<std::string, int> deaths_;
  std::unordered_map<std::string, std::unordered_map<std::string, double>> experience_gained_;
  int encounters_;
  std::unordered_map<std::string, 
                   std::unordered_map<std::string, 
                                    std::unordered_map<std::string, 
                                                     std::unordered_map<std::string, int>>>> attacks_;
  std::unordered_map<std::string, std::unordered_map<std::string, int>> consumables_used_;
  std::unordered_map<std::string, std::unordered_map<std::string, double>> hitpoints_gained_;
  std::unordered_map<std::string, std::unordered_map<std::string, double>> manapoints_gained_;
  std::unordered_map<std::string, double> drop_rate_multiplier_;
  std::unordered_map<std::string, double> rare_find_multiplier_;
  std::unordered_map<std::string, bool> player_ran_out_of_mana_;
  std::unordered_map<std::string, std::vector<int>> player_ran_out_of_mana_time_;
  std::unordered_map<std::string, std::unordered_map<std::string, double>> mana_used_;
  std::vector<TimeAliveInfo> time_spent_alive_;
  std::vector<BossSpawn> boss_spawns_;
  int elite_tier_;
  std::unordered_map<std::string, std::unordered_map<std::string, double>> hitpoints_spent_;
  std::string zone_name_;
  bool is_dungeon_;
  int dungeons_completed_;
  int dungeons_failed_;
  int max_wave_reached_;
  int number_of_players_;
};

}  // namespace combat_simulator_overhaul

#endif  // COMBAT_SIMULATOR_OVERHAUL_SIM_RESULT_H_
