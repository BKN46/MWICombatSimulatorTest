// Copyright 2025 BKNMWICombatSimulator
//
// combat_simulator.cc - Main combat simulation engine implementation.

#include "combat_simulator.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "player.h"
#include "monster.h"
#include "zone.h"
#include "events/combat_events.h"
#include "events/event_queue.h"
#include "sim_result.h"

namespace combat_simulator {

namespace {
constexpr double kOneSecond = 1e9;
constexpr double kHotTickInterval = 5 * kOneSecond;
constexpr double kDotTickInterval = 3 * kOneSecond;
constexpr double kRegenTickInterval = 10 * kOneSecond;
constexpr double kEnemyRespawnInterval = 3 * kOneSecond;
constexpr double kPlayerRespawnInterval = 150 * kOneSecond;
constexpr double kRestartInterval = 15 * kOneSecond;
}

// 获取单位的食物和饮品（通过虚函数，基类返回空，Player/Monster重载返回实际容器）
namespace {
const std::vector<std::shared_ptr<Consumable>>& GetUnitFood(const std::shared_ptr<CombatUnit>& unit) {
  if (unit->is_player()) {
    return static_cast<Player*>(unit.get())->food();
  }
  static const std::vector<std::shared_ptr<Consumable>> empty;
  return empty;
}
const std::vector<std::shared_ptr<Consumable>>& GetUnitDrinks(const std::shared_ptr<CombatUnit>& unit) {
  if (unit->is_player()) {
    return static_cast<Player*>(unit.get())->drinks();
  }
  static const std::vector<std::shared_ptr<Consumable>> empty;
  return empty;
}
}

CombatSimulator::CombatSimulator(const std::vector<std::shared_ptr<Player>>& players,
                                 std::shared_ptr<Zone> zone,
                                 const std::string& sim_id)
    : players_(players),
      zone_(std::move(zone)),
      event_queue_(std::make_unique<EventQueue>()),
      sim_id_(sim_id.empty() ? std::to_string(rand()) : sim_id),
      sim_result_(std::make_shared<SimResult>(zone_->hrid(), players_.size(), sim_id_)),
      all_players_dead_(false),
      simulation_time_(0) {}

std::shared_ptr<SimResult> CombatSimulator::Simulate(double simulation_time_limit) {
  Reset();
  int ticks = 0;
  event_queue_->AddEvent(new events::CombatStartEvent(0));
  while (simulation_time_ < simulation_time_limit) {
    auto next_event = event_queue_->GetNextEvent();
    if (!next_event) break;
    ProcessEvent(next_event);
    ticks++;
    // 可加进度回调
  }
  // 结果处理略，详见JS实现
  return sim_result_;
}

void CombatSimulator::Reset() {
  simulation_time_ = 0;
  all_players_dead_ = false;
  event_queue_->Clear();
  sim_result_ = std::make_shared<SimResult>(zone_->hrid(), players_.size(), sim_id_);
}

void CombatSimulator::ProcessEvent(Event* event) {
  simulation_time_ = event->time();
  using namespace events;
  if (event->type() == CombatStartEvent::kType) {
    ProcessCombatStartEvent(static_cast<CombatStartEvent*>(event));
  } else if (event->type() == PlayerRespawnEvent::kType) {
    ProcessPlayerRespawnEvent(static_cast<PlayerRespawnEvent*>(event));
  } else if (event->type() == EnemyRespawnEvent::kType) {
    ProcessEnemyRespawnEvent(static_cast<EnemyRespawnEvent*>(event));
  } else if (event->type() == AutoAttackEvent::kType) {
    ProcessAutoAttackEvent(static_cast<AutoAttackEvent*>(event));
  } else if (event->type() == ConsumableTickEvent::kType) {
    ProcessConsumableTickEvent(static_cast<ConsumableTickEvent*>(event));
  } else if (event->type() == DamageOverTimeEvent::kType) {
    ProcessDamageOverTimeTickEvent(static_cast<DamageOverTimeEvent*>(event));
  } else if (event->type() == CheckBuffExpirationEvent::kType) {
    ProcessCheckBuffExpirationEvent(static_cast<CheckBuffExpirationEvent*>(event));
  } else if (event->type() == RegenTickEvent::kType) {
    ProcessRegenTickEvent(static_cast<RegenTickEvent*>(event));
  } else if (event->type() == StunExpirationEvent::kType) {
    ProcessStunExpirationEvent(static_cast<StunExpirationEvent*>(event));
  } else if (event->type() == BlindExpirationEvent::kType) {
    ProcessBlindExpirationEvent(static_cast<BlindExpirationEvent*>(event));
  } else if (event->type() == SilenceExpirationEvent::kType) {
    ProcessSilenceExpirationEvent(static_cast<SilenceExpirationEvent*>(event));
  } else if (event->type() == CurseExpirationEvent::kType) {
    ProcessCurseExpirationEvent(static_cast<CurseExpirationEvent*>(event));
  } else if (event->type() == WeakenExpirationEvent::kType) {
    ProcessWeakenExpirationEvent(static_cast<WeakenExpirationEvent*>(event));
  } else if (event->type() == FuryExpirationEvent::kType) {
    ProcessFuryExpirationEvent(static_cast<FuryExpirationEvent*>(event));
  } else if (event->type() == AbilityCastEndEvent::kType) {
    ProcessAbilityCastEndEvent(static_cast<AbilityCastEndEvent*>(event));
  } else if (event->type() == AwaitCooldownEvent::kType) {
    ProcessAwaitCooldownEvent(static_cast<AwaitCooldownEvent*>(event));
  } else if (event->type() == CooldownReadyEvent::kType) {
    // 仅用于触发检查，无操作
  }
  // 触发检查
  CheckTriggers();
}

void CombatSimulator::StartNewEncounter() {
  // ...实现同JS逻辑...
}

void CombatSimulator::AddNextAttackEvent(std::shared_ptr<CombatUnit> source) {
  // ...实现同JS逻辑...
}

bool CombatSimulator::CheckEncounterEnd() {
  // ...实现同JS逻辑...
  return false;
}

bool CombatSimulator::TryUseAbility(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability) {
  // ...实现同JS逻辑...
  return false;
}

bool CombatSimulator::TryUseConsumable(std::shared_ptr<CombatUnit> source, std::shared_ptr<Consumable> consumable) {
  // ...实现同JS逻辑...
  return false;
}

void CombatSimulator::ProcessCombatStartEvent(events::CombatStartEvent* event) {}
void CombatSimulator::ProcessPlayerRespawnEvent(events::PlayerRespawnEvent* event) {}
void CombatSimulator::ProcessEnemyRespawnEvent(events::EnemyRespawnEvent* event) {}
void CombatSimulator::ProcessAutoAttackEvent(events::AutoAttackEvent* event) {}
void CombatSimulator::ProcessConsumableTickEvent(events::ConsumableTickEvent* event) {}
void CombatSimulator::ProcessDamageOverTimeTickEvent(events::DamageOverTimeEvent* event) {}
void CombatSimulator::ProcessCheckBuffExpirationEvent(events::CheckBuffExpirationEvent* event) {}
void CombatSimulator::ProcessRegenTickEvent(events::RegenTickEvent* event) {}
void CombatSimulator::ProcessStunExpirationEvent(events::StunExpirationEvent* event) {}
void CombatSimulator::ProcessBlindExpirationEvent(events::BlindExpirationEvent* event) {}
void CombatSimulator::ProcessSilenceExpirationEvent(events::SilenceExpirationEvent* event) {}
void CombatSimulator::ProcessCurseExpirationEvent(events::CurseExpirationEvent* event) {}
void CombatSimulator::ProcessWeakenExpirationEvent(events::WeakenExpirationEvent* event) {}
void CombatSimulator::ProcessFuryExpirationEvent(events::FuryExpirationEvent* event) {}
void CombatSimulator::ProcessAbilityCastEndEvent(events::AbilityCastEndEvent* event) {}
void CombatSimulator::ProcessAwaitCooldownEvent(events::AwaitCooldownEvent* event) {}

void CombatSimulator::CheckTriggers() {
  bool triggered_something;
  do {
    triggered_something = false;
    // 玩家
    std::vector<std::shared_ptr<CombatUnit>> player_units;
    for (const auto& player : players_) {
      player_units.push_back(std::static_pointer_cast<CombatUnit>(player));
    }
    std::vector<std::shared_ptr<CombatUnit>> enemy_units;
    for (const auto& enemy : enemies_) {
      enemy_units.push_back(std::static_pointer_cast<CombatUnit>(enemy));
    }
    for (const auto& player : players_) {
      if (player->combat_details().current_hitpoints > 0) {
        if (CheckTriggersForUnit(std::static_pointer_cast<CombatUnit>(player), player_units, enemy_units)) {
          triggered_something = true;
        }
      }
    }
    for (const auto& enemy : enemies_) {
      if (enemy->combat_details().current_hitpoints > 0) {
        if (CheckTriggersForUnit(std::static_pointer_cast<CombatUnit>(enemy), enemy_units, player_units)) {
          triggered_something = true;
        }
      }
    }
  } while (triggered_something);
}

bool CombatSimulator::CheckTriggersForUnit(
    const std::shared_ptr<CombatUnit>& unit,
    const std::vector<std::shared_ptr<CombatUnit>>& friendlies,
    const std::vector<std::shared_ptr<CombatUnit>>& enemies) {
  if (unit->combat_details().current_hitpoints <= 0) {
    return false;
  }
  bool triggered_something = false;
  for (const auto& food : GetUnitFood(unit)) {
    if (food && TryUseConsumable(unit, food)) {
      triggered_something = true;
    }
  }
  for (const auto& drink : GetUnitDrinks(unit)) {
    if (drink && TryUseConsumable(unit, drink)) {
      triggered_something = true;
    }
  }
  return triggered_something;
}

}  // namespace combat_simulator
