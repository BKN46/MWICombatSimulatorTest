#include "zone.h"
#include <algorithm>
#include <random>
#include <iostream>
#include "combat_simulator_overhaul/data/action_detail_map.h" // Assuming this is created as C++ header

namespace combat_simulator_overhaul {

Zone::Zone(const std::string& hrid) : hrid_(hrid) {
  auto game_zone = GetActionDetail(hrid_);
  if (!game_zone) {
    throw std::runtime_error("No zone found for hrid: " + hrid_);
  }
  
  monster_spawn_info_ = game_zone->combat_zone_info.fight_info;
  dungeon_spawn_info_ = game_zone->combat_zone_info.dungeon_info;
  encounters_killed_ = 1;
  
  // Set battles per boss
  SetValue(monster_spawn_info_, "battles_per_boss", 10);
  
  buffs_ = game_zone->buffs;
  is_dungeon_ = game_zone->combat_zone_info.is_dungeon;
  dungeons_completed_ = 0;
  dungeons_failed_ = 0;
  final_wave_ = false;
}

std::vector<std::shared_ptr<Monster>> Zone::GetRandomEncounter() {
  std::vector<std::shared_ptr<Monster>> monsters;
  
  // Check for boss spawn
  auto boss_spawns = GetValue<std::vector<std::unordered_map<std::string, std::any>>>(
      monster_spawn_info_, "boss_spawns");
      
  int battles_per_boss = GetValue<int>(monster_spawn_info_, "battles_per_boss");
  
  if (boss_spawns.size() > 0 && encounters_killed_ == battles_per_boss) {
    encounters_killed_ = 1;
    
    for (const auto& monster : boss_spawns) {
      std::string hrid = GetValue<std::string>(monster, "combat_monster_hrid");
      int elite_tier = GetValue<int>(monster, "elite_tier");
      monsters.push_back(std::make_shared<Monster>(hrid, elite_tier));
    }
    return monsters;
  }
  
  // Handle random spawns
  auto random_spawn_info = GetValue<std::unordered_map<std::string, std::any>>(
      monster_spawn_info_, "random_spawn_info");
      
  auto spawns = GetValue<std::vector<std::unordered_map<std::string, std::any>>>(
      random_spawn_info, "spawns");
      
  int max_spawn_count = GetValue<int>(random_spawn_info, "max_spawn_count");
  int max_total_strength = GetValue<int>(random_spawn_info, "max_total_strength");
  
  // Calculate total weight
  double total_weight = 0.0;
  for (const auto& spawn : spawns) {
    total_weight += GetValue<double>(spawn, "rate");
  }
  
  // Select random monsters
  std::vector<std::pair<std::string, int>> encounter_hrids;
  int total_strength = 0;
  
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.0, 1.0);
  
  for (int i = 0; i < max_spawn_count; ++i) {
    double random_weight = total_weight * dis(gen);
    double cumulative_weight = 0.0;
    
    for (const auto& spawn : spawns) {
      cumulative_weight += GetValue<double>(spawn, "rate");
      
      if (random_weight <= cumulative_weight) {
        int strength = GetValue<int>(spawn, "strength");
        total_strength += strength;
        
        if (total_strength <= max_total_strength) {
          std::string hrid = GetValue<std::string>(spawn, "combat_monster_hrid");
          int elite_tier = GetValue<int>(spawn, "elite_tier");
          encounter_hrids.emplace_back(hrid, elite_tier);
        } else {
          // Break outer loop
          i = max_spawn_count;
        }
        break;
      }
    }
  }
  
  encounters_killed_++;
  
  // Create monster objects
  for (const auto& [hrid, elite_tier] : encounter_hrids) {
    monsters.push_back(std::make_shared<Monster>(hrid, elite_tier));
  }
  
  return monsters;
}

void Zone::FailWave() {
  dungeons_failed_++;
  encounters_killed_ = 1;
}

std::vector<std::shared_ptr<Monster>> Zone::GetNextWave() {
  // Check if we've completed all waves
  int max_waves = GetValue<int>(dungeon_spawn_info_, "max_waves");
  
  if (encounters_killed_ > max_waves) {
    dungeons_completed_++;
    encounters_killed_ = 1;
  }
  
  // Check for fixed spawns first
  auto fixed_spawns_map = GetValue<std::unordered_map<std::string, std::any>>(
      dungeon_spawn_info_, "fixed_spawns_map");
      
  std::string current_wave = std::to_string(encounters_killed_);
  
  if (fixed_spawns_map.find(current_wave) != fixed_spawns_map.end()) {
    auto current_monsters = GetValue<std::vector<std::unordered_map<std::string, std::any>>>(
        fixed_spawns_map, current_wave);
        
    encounters_killed_++;
    
    std::vector<std::shared_ptr<Monster>> monsters;
    for (const auto& monster : current_monsters) {
      std::string hrid = GetValue<std::string>(monster, "combat_monster_hrid");
      int elite_tier = GetValue<int>(monster, "elite_tier");
      monsters.push_back(std::make_shared<Monster>(hrid, elite_tier));
    }
    
    return monsters;
  } else {
    // Handle random spawns
    auto random_spawn_info_map = GetValue<std::unordered_map<std::string, std::any>>(
        dungeon_spawn_info_, "random_spawn_info_map");
    
    // Get all wave keys and sort them
    std::vector<int> wave_keys;
    for (const auto& [key, _] : random_spawn_info_map) {
      wave_keys.push_back(std::stoi(key));
    }
    std::sort(wave_keys.begin(), wave_keys.end());
    
    std::unordered_map<std::string, std::any> monster_spawns;
    
    if (encounters_killed_ > wave_keys.back()) {
      monster_spawns = GetValue<std::unordered_map<std::string, std::any>>(
          random_spawn_info_map, std::to_string(wave_keys.back()));
    } else {
      for (size_t i = 0; i < wave_keys.size() - 1; ++i) {
        if (encounters_killed_ >= wave_keys[i] && encounters_killed_ <= wave_keys[i + 1]) {
          monster_spawns = GetValue<std::unordered_map<std::string, std::any>>(
              random_spawn_info_map, std::to_string(wave_keys[i]));
          break;
        }
      }
    }
    
    auto spawns = GetValue<std::vector<std::unordered_map<std::string, std::any>>>(
        monster_spawns, "spawns");
    
    int max_spawn_count = GetValue<int>(monster_spawns, "max_spawn_count");
    int max_total_strength = GetValue<int>(monster_spawns, "max_total_strength");
    
    // Calculate total weight
    double total_weight = 0.0;
    for (const auto& spawn : spawns) {
      total_weight += GetValue<double>(spawn, "rate");
    }
    
    // Select random monsters
    std::vector<std::pair<std::string, int>> encounter_hrids;
    int total_strength = 0;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    for (int i = 0; i < max_spawn_count; ++i) {
      double random_weight = total_weight * dis(gen);
      double cumulative_weight = 0.0;
      
      for (const auto& spawn : spawns) {
        cumulative_weight += GetValue<double>(spawn, "rate");
        
        if (random_weight <= cumulative_weight) {
          int strength = GetValue<int>(spawn, "strength");
          total_strength += strength;
          
          if (total_strength <= max_total_strength) {
            std::string hrid = GetValue<std::string>(spawn, "combat_monster_hrid");
            int elite_tier = GetValue<int>(spawn, "elite_tier");
            encounter_hrids.emplace_back(hrid, elite_tier);
          } else {
            // Break outer loop
            i = max_spawn_count;
          }
          break;
        }
      }
    }
    
    encounters_killed_++;
    
    // Create monster objects
    std::vector<std::shared_ptr<Monster>> monsters;
    for (const auto& [hrid, elite_tier] : encounter_hrids) {
      monsters.push_back(std::make_shared<Monster>(hrid, elite_tier));
    }
    
    return monsters;
  }
}

} // namespace combat_simulator_overhaul
