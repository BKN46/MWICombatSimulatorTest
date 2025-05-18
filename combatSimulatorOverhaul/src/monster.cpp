#include "monster.h"

#include <stdexcept>
#include "combat_monster_detail_map.h" // Assuming this provides access to the JSON data

Monster::Monster(const std::string& hrid, int elite_tier) 
    : CombatUnit(), elite_tier_(elite_tier) {
    
    is_player_ = false;
    hrid_ = hrid;
    
    auto game_monster = GetCombatMonsterDetail(hrid_);
    if (!game_monster) {
        throw std::runtime_error("No monster found for hrid: " + hrid_);
    }

    // Process abilities
    for (size_t i = 0; i < game_monster->abilities.size(); i++) {
        if (game_monster->abilities[i].min_elite_tier > elite_tier_) {
            continue;
        }
        abilities_.push_back(std::make_shared<Ability>(
            game_monster->abilities[i].ability_hrid, 
            game_monster->abilities[i].level));
    }
    
    // Process drop table if it exists
    if (!game_monster->drop_table.empty()) {
        for (size_t i = 0; i < game_monster->drop_table.size(); i++) {
            drop_table_.push_back(std::make_shared<Drops>(
                game_monster->drop_table[i].item_hrid, 
                game_monster->drop_table[i].drop_rate, 
                game_monster->drop_table[i].min_count, 
                game_monster->drop_table[i].max_count, 
                game_monster->drop_table[i].elite_tier));
        }
    }
    
    // Process rare drop table
    for (size_t i = 0; i < game_monster->rare_drop_table.size(); i++) {
        const auto& drop_table_item = (i < game_monster->drop_table.size()) ? 
                                      &game_monster->drop_table[i] : nullptr;
        
        int item_elite_tier = drop_table_item ? 
                              drop_table_item->elite_tier : 
                              game_monster->rare_drop_table[i].min_elite_tier;
        
        rare_drop_table_.push_back(std::make_shared<Drops>(
            game_monster->rare_drop_table[i].item_hrid, 
            game_monster->rare_drop_table[i].drop_rate, 
            game_monster->rare_drop_table[i].min_count, 
            item_elite_tier));
    }
}

void Monster::UpdateCombatDetails() {
    auto game_monster = GetCombatMonsterDetail(hrid_);
    
    // Define the list of stats to initialize
    std::vector<std::string> stats_to_initialize = {
        "stab_accuracy", "slash_accuracy", "smash_accuracy", "ranged_accuracy", 
        "magic_accuracy", "stab_damage", "slash_damage", "smash_damage", 
        "ranged_damage", "magic_damage", "task_damage", "physical_amplify", 
        "water_amplify", "nature_amplify", "fire_amplify", "healing_amplify", 
        "stab_evasion", "slash_evasion", "smash_evasion", "ranged_evasion", 
        "magic_evasion", "armor", "water_resistance", "nature_resistance", 
        "fire_resistance", "max_hitpoints", "max_manapoints", "life_steal", 
        "hp_regen_per_10", "mp_regen_per_10", "physical_thorns", "elemental_thorns", 
        "combat_drop_rate", "combat_rare_find", "combat_drop_quantity", 
        "combat_experience", "critical_rate", "critical_damage", "armor_penetration", 
        "water_penetration", "nature_penetration", "fire_penetration", 
        "ability_haste", "tenacity", "mana_leech", "cast_speed", "threat", 
        "parry", "mayhem", "pierce", "curse", "fury", "weaken", "ripple", 
        "bloom", "blaze", "attack_speed", "food_haste", "drink_concentration", 
        "auto_attack_damage"
    };
    
    // Apply stats based on elite tier
    const auto* combat_details = nullptr;
    
    switch (elite_tier_) {
        case 2:
            stamina_level_ = game_monster->elite2_combat_details.stamina_level;
            intelligence_level_ = game_monster->elite2_combat_details.intelligence_level;
            attack_level_ = game_monster->elite2_combat_details.attack_level;
            power_level_ = game_monster->elite2_combat_details.power_level;
            defense_level_ = game_monster->elite2_combat_details.defense_level;
            ranged_level_ = game_monster->elite2_combat_details.ranged_level;
            magic_level_ = game_monster->elite2_combat_details.magic_level;
            
            combat_details_.combat_stats.combat_style_hrid = 
                game_monster->elite2_combat_details.combat_stats.combat_style_hrids[0];
            
            // Copy all stats from elite2 combat details
            CopyStats(game_monster->elite2_combat_details.combat_stats, 
                     combat_details_.combat_stats);
            
            combat_details = &game_monster->elite2_combat_details;
            break;
            
        case 1:
            stamina_level_ = game_monster->elite1_combat_details.stamina_level;
            intelligence_level_ = game_monster->elite1_combat_details.intelligence_level;
            attack_level_ = game_monster->elite1_combat_details.attack_level;
            power_level_ = game_monster->elite1_combat_details.power_level;
            defense_level_ = game_monster->elite1_combat_details.defense_level;
            ranged_level_ = game_monster->elite1_combat_details.ranged_level;
            magic_level_ = game_monster->elite1_combat_details.magic_level;
            
            combat_details_.combat_stats.combat_style_hrid = 
                game_monster->elite1_combat_details.combat_stats.combat_style_hrids[0];
            
            // Copy all stats from elite1 combat details
            CopyStats(game_monster->elite1_combat_details.combat_stats, 
                     combat_details_.combat_stats);
                     
            combat_details = &game_monster->elite1_combat_details;
            break;
            
        default:
            stamina_level_ = game_monster->combat_details.stamina_level;
            intelligence_level_ = game_monster->combat_details.intelligence_level;
            attack_level_ = game_monster->combat_details.attack_level;
            power_level_ = game_monster->combat_details.power_level;
            defense_level_ = game_monster->combat_details.defense_level;
            ranged_level_ = game_monster->combat_details.ranged_level;
            magic_level_ = game_monster->combat_details.magic_level;
            
            combat_details_.combat_stats.combat_style_hrid = 
                game_monster->combat_details.combat_stats.combat_style_hrids[0];
                
            // Copy all stats from standard combat details
            CopyStats(game_monster->combat_details.combat_stats, 
                     combat_details_.combat_stats);
                     
            combat_details = &game_monster->combat_details;
            break;
    }
    
    // Initialize any missing stats to 0
    for (const auto& stat : stats_to_initialize) {
        if (!HasStat(combat_details_.combat_stats, stat)) {
            SetStat(combat_details_.combat_stats, stat, 0);
        }
    }
    
    combat_details_.combat_stats.attack_interval = combat_details->attack_interval;
    
    // Call parent class implementation
    CombatUnit::UpdateCombatDetails();
}
