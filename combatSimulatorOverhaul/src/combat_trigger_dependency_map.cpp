#include "combat_trigger_dependency_map.h"
#include <fstream>
#include <nlohmann/json.hpp> // Using nlohmann/json for JSON parsing

std::unordered_map<std::string, TriggerDependencyDetail> CombatTriggerDependencyMap::map_;
bool CombatTriggerDependencyMap::initialized_ = false;

std::unordered_map<std::string, TriggerDependencyDetail>& CombatTriggerDependencyMap::Get() {
    if (!initialized_) {
        Initialize();
    }
    return map_;
}

void CombatTriggerDependencyMap::Initialize() {
    // Load the JSON file
    std::ifstream file("data/combat_trigger_dependency_detail_map.json");
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open combat_trigger_dependency_detail_map.json");
    }

    nlohmann::json json_data;
    file >> json_data;

    // Parse the JSON data into our map
    for (auto& item : json_data.items()) {
        std::string key = item.key();
        auto& value = item.value();
        
        TriggerDependencyDetail detail;
        detail.is_single_target = value["isSingleTarget"];
        // Add other properties as needed
        map_[key] = detail;
    }

    initialized_ = true;
}
