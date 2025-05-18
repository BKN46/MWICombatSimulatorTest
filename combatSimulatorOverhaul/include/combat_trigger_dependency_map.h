#pragma once

#include <string>
#include <unordered_map>

// This class represents the JSON data from combat_trigger_dependency_detail_map.json
struct TriggerDependencyDetail {
    bool is_single_target;
    // Add other properties from the JSON if needed
};

class CombatTriggerDependencyMap {
public:
    static std::unordered_map<std::string, TriggerDependencyDetail>& Get();

private:
    static std::unordered_map<std::string, TriggerDependencyDetail> map_;
    static bool initialized_;
    static void Initialize();
};
