// Copyright 2025 BKNMWICombatSimulator
//
// combat_simulator.h - Main combat simulation engine.

#ifndef COMBAT_SIMULATOR_OVERHAUL_INCLUDE_COMBAT_SIMULATOR_H_
#define COMBAT_SIMULATOR_OVERHAUL_INCLUDE_COMBAT_SIMULATOR_H_

#include <memory>
#include <string>
#include <vector>

#include "player.h"
#include "monster.h"
#include "zone.h"
#include "events/combat_events.h"
#include "events/event_queue.h"
#include "sim_result.h"

namespace combat_simulator {

class CombatSimulator {
 public:
  CombatSimulator(const std::vector<std::shared_ptr<Player>>& players,
                  std::shared_ptr<Zone> zone,
                  const std::string& sim_id = "");

  // 主仿真入口
  std::shared_ptr<SimResult> Simulate(double simulation_time_limit);

  // 重置仿真状态
  void Reset();

 private:
  void ProcessEvent(Event* event);
  void StartNewEncounter();
  void AddNextAttackEvent(std::shared_ptr<CombatUnit> source);
  bool CheckEncounterEnd();
  bool TryUseAbility(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability);
  bool TryUseConsumable(std::shared_ptr<CombatUnit> source, std::shared_ptr<Consumable> consumable);

  // 事件处理分派
  void ProcessCombatStartEvent(events::CombatStartEvent* event);
  void ProcessPlayerRespawnEvent(events::PlayerRespawnEvent* event);
  void ProcessEnemyRespawnEvent(events::EnemyRespawnEvent* event);
  void ProcessAutoAttackEvent(events::AutoAttackEvent* event);
  void ProcessConsumableTickEvent(events::ConsumableTickEvent* event);
  void ProcessDamageOverTimeTickEvent(events::DamageOverTimeEvent* event);
  void ProcessCheckBuffExpirationEvent(events::CheckBuffExpirationEvent* event);
  void ProcessRegenTickEvent(events::RegenTickEvent* event);
  void ProcessStunExpirationEvent(events::StunExpirationEvent* event);
  void ProcessBlindExpirationEvent(events::BlindExpirationEvent* event);
  void ProcessSilenceExpirationEvent(events::SilenceExpirationEvent* event);
  void ProcessCurseExpirationEvent(events::CurseExpirationEvent* event);
  void ProcessWeakenExpirationEvent(events::WeakenExpirationEvent* event);
  void ProcessFuryExpirationEvent(events::FuryExpirationEvent* event);
  void ProcessAbilityCastEndEvent(events::AbilityCastEndEvent* event);
  void ProcessAwaitCooldownEvent(events::AwaitCooldownEvent* event);

  void CheckTriggers();
  bool CheckTriggersForUnit(const std::shared_ptr<CombatUnit>& unit,
    const std::vector<std::shared_ptr<CombatUnit>>& friendlies,
    const std::vector<std::shared_ptr<CombatUnit>>& enemies);

  // 主要成员
  std::vector<std::shared_ptr<Player>> players_;
  std::vector<std::shared_ptr<Monster>> enemies_;
  std::shared_ptr<Zone> zone_;
  std::unique_ptr<EventQueue> event_queue_;
  std::shared_ptr<SimResult> sim_result_;
  std::string sim_id_;
  double simulation_time_ = 0;
  bool all_players_dead_ = false;
};

}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_OVERHAUL_INCLUDE_COMBAT_SIMULATOR_H_
