// Copyright 2025 BKNMWICombatSimulator
//
// trigger.h - Definition of the Trigger class which represents a condition
// for ability activation in the combat system.

#ifndef COMBAT_SIMULATOR_TRIGGER_H_
#define COMBAT_SIMULATOR_TRIGGER_H_

#include <string>
#include <vector>
#include <memory>

#include "nlohmann/json.hpp"

namespace combat_simulator {

// Forward declarations
class CombatUnit;

// Represents a trigger condition for abilities in the combat system.
class Trigger {
 public:
  // Default constructor
  Trigger() = default;
  
  // Constructs a trigger with the specified parameters
  Trigger(const std::string& dependency_hrid, 
         const std::string& condition_hrid,
         const std::string& comparator_hrid,
         double value = 0.0);
  
  // Creates a trigger from a data transfer object (JSON)
  static std::unique_ptr<Trigger> CreateFromDTO(const nlohmann::json& dto);
  
  // Checks if the trigger is active for the given combat situation
  bool IsActive(const CombatUnit& source, 
               const CombatUnit* target,
               const std::vector<std::shared_ptr<CombatUnit>>& friendlies,
               const std::vector<std::shared_ptr<CombatUnit>>& enemies,
               int64_t current_time) const;
  
  // Getters
  const std::string& dependency_hrid() const { return dependency_hrid_; }
  const std::string& condition_hrid() const { return condition_hrid_; }
  const std::string& comparator_hrid() const { return comparator_hrid_; }
  double value() const { return value_; }
  
 private:
  // Checks if the trigger is active for a single target situation
  bool IsActiveSingleTarget(const CombatUnit& source,
                          const CombatUnit* target,
                          int64_t current_time) const;
  
  // Checks if the trigger is active for a multi-target situation
  bool IsActiveMultiTarget(const std::vector<std::shared_ptr<CombatUnit>>& friendlies,
                         const std::vector<std::shared_ptr<CombatUnit>>& enemies,
                         int64_t current_time) const;
  
  // Gets the value of the dependency for a given unit
  double GetDependencyValue(const CombatUnit& source, int64_t current_time) const;
  
  // Compares the dependency value with the trigger value according to the comparator
  bool CompareValue(double dependency_value) const;
  
  // Member variables
  std::string dependency_hrid_;
  std::string condition_hrid_;
  std::string comparator_hrid_;
  double value_ = 0.0;
};

}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_TRIGGER_H_
