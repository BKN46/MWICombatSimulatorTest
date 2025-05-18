#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <limits>
#include <cmath>
#include <algorithm>

namespace combat_simulator_overhaul {

// Forward declarations
struct Buff;
struct Ability;
struct Food;
struct Drink;
struct HouseRoom;

// Combat stats structure
struct CombatStats {
  std::string combat_style_hrid;
  std::string damage_type;
  int64_t attack_interval;
  double auto_attack_damage;
  double critical_rate;
  double critical_damage;
  double stab_accuracy;
  double slash_accuracy;
  double smash_accuracy;
  double ranged_accuracy;
  double magic_accuracy;
  double stab_damage;
  double slash_damage;
  double smash_damage;
  double ranged_damage;
  double magic_damage;
  double task_damage;
  double physical_amplify;
  double water_amplify;
  double nature_amplify;
  double fire_amplify;
  double healing_amplify;
  double physical_thorns;
  double elemental_thorns;
  double max_hitpoints;
  double max_manapoints;
  double stab_evasion;
  double slash_evasion;
  double smash_evasion;
  double ranged_evasion;
  double magic_evasion;
  double armor;
  double water_resistance;
  double nature_resistance;
  double fire_resistance;
  double life_steal;
  double hp_regen_per_10;
  double mp_regen_per_10;
  double combat_drop_rate;
  double combat_drop_quantity;
  double combat_rare_find;
  double combat_experience;
  double food_slots;
  double drink_slots;
  double armor_penetration;
  double water_penetration;
  double nature_penetration;
  double fire_penetration;
  double mana_leech;
  double cast_speed;
  double threat;
  double parry;
  double mayhem;
  double pierce;
  double curse;
  double ripple;
  double bloom;
  double blaze;
  double weaken;
  double fury;
  double food_haste;
  double drink_concentration;
  double damage_taken;
  double attack_speed;
  double armor_damage_ratio;
  double hp_drain_ratio;

  CombatStats();
};

// Combat details structure
struct CombatDetails {
  double stamina_level;
  double intelligence_level;
  double attack_level;
  double power_level;
  double defense_level;
  double ranged_level;
  double magic_level;
  double max_hitpoints;
  double current_hitpoints;
  double max_manapoints;
  double current_manapoints;
  double stab_accuracy_rating;
  double slash_accuracy_rating;
  double smash_accuracy_rating;
  double ranged_accuracy_rating;
  double magic_accuracy_rating;
  double stab_max_damage;
  double slash_max_damage;
  double smash_max_damage;
  double ranged_max_damage;
  double magic_max_damage;
  double stab_evasion_rating;
  double slash_evasion_rating;
  double smash_evasion_rating;
  double ranged_evasion_rating;
  double magic_evasion_rating;
  double total_armor;
  double total_water_resistance;
  double total_nature_resistance;
  double total_fire_resistance;
  double ability_haste;
  double tenacity;
  double total_threat;
  CombatStats combat_stats;

  CombatDetails();
};

// Buff boost structure
struct BuffBoost {
  double ratio_boost;
  double flat_boost;
  
  BuffBoost() : ratio_boost(0), flat_boost(0) {}
};

// Buff structure
struct Buff {
  std::string unique_hrid;
  std::string type_hrid;
  int64_t start_time;
  int64_t duration;
  double ratio_boost;
  double flat_boost;
};

class CombatUnit {
 public:
  CombatUnit();
  
  void update_combat_details();
  void add_curse(double curse);
  double update_fury(bool is_hit, double fury);
  void add_buff(const Buff& buff, int64_t current_time);
  void remove_buff(const Buff& buff);
  void add_permanent_buff(const Buff& buff);
  void generate_permanent_buffs();
  void remove_expired_buffs(int64_t current_time);
  void clear_buffs();
  void clear_ccs();
  std::vector<BuffBoost> get_buff_boosts(const std::string& type);
  BuffBoost get_buff_boost(const std::string& type);
  void reset(int64_t current_time = 0);
  void reset_cooldowns(int64_t current_time = 0);
  double add_hitpoints(double hitpoints);
  double add_manapoints(double manapoints);

 public:  // In real code, you might want to make these private with getters/setters
  bool is_player;
  bool is_stunned;
  int64_t stun_expire_time;
  bool is_blinded;
  int64_t blind_expire_time;
  bool is_silenced;
  int64_t silence_expire_time;
  double curse_value;
  double fury_value;
  bool is_weakened;
  int64_t weaken_expire_time;
  double weaken_percentage;

  // Base levels
  double stamina_level;
  double intelligence_level;
  double attack_level;
  double power_level;
  double defense_level;
  double ranged_level;
  double magic_level;

  // Equipment and abilities
  std::vector<std::shared_ptr<Ability>> abilities;
  std::vector<std::shared_ptr<Food>> food;
  std::vector<std::shared_ptr<Drink>> drinks;
  std::vector<std::shared_ptr<HouseRoom>> house_rooms;
  std::vector<std::shared_ptr<void>> drop_table;
  std::vector<std::shared_ptr<void>> rare_drop_table;
  std::map<std::string, double> ability_mana_costs;

  // Combat details and buffs
  CombatDetails combat_details;
  std::map<std::string, Buff> combat_buffs;
  std::map<std::string, Buff> permanent_buffs;
  std::vector<Buff> zone_buffs;
};

}  // namespace combat_simulator_overhaul