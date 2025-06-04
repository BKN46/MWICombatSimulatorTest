// Copyright 2025 BKNMWICombatSimulator
//
// combat_simulator.cc - Main combat simulation engine implementation.

#include "combat_simulator.h"

#include <algorithm>
#include <cmath>
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
#include "combat_utilities.h"
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
  if (all_players_dead_) {
    all_players_dead_ = false;
    zone_->FailWave();
  }
  if (!zone_->is_dungeon()) {
    enemies_ = zone_->GetRandomEncounter();
  } else {
    enemies_ = zone_->GetNextWave();
    // 可选: sim_result_->UpdateTimeSpentAlive("#" + std::to_string(zone_->encounters_killed() - 1), true, simulation_time_);
    // 可选: 统计地牢完成数等
  }
  for (const auto& enemy : enemies_) {
    enemy->Reset(simulation_time_);
    // 可选: sim_result_->UpdateTimeSpentAlive(enemy->hrid(), true, simulation_time_);
  }
  StartAttacks();
}

void CombatSimulator::StartAttacks() {
  std::vector<std::shared_ptr<CombatUnit>> units;
  for (const auto& p : players_) units.push_back(std::static_pointer_cast<CombatUnit>(p));
  for (const auto& e : enemies_) units.push_back(std::static_pointer_cast<CombatUnit>(e));
  for (const auto& unit : units) {
    if (unit->combat_details().current_hitpoints <= 0) continue;
    AddNextAttackEvent(unit);
  }
}

void CombatSimulator::AddNextAttackEvent(std::shared_ptr<CombatUnit> source) {
  std::shared_ptr<CombatUnit> target;
  std::vector<std::shared_ptr<CombatUnit>> friendlies, enemies;
  if (source->is_player()) {
    std::vector<std::shared_ptr<CombatUnit>> enemy_units;
    for (const auto& e : enemies_) enemy_units.push_back(std::static_pointer_cast<CombatUnit>(e));
    target = combat_simulator::CombatUtilities::GetTarget(enemy_units);
    for (const auto& p : players_) friendlies.push_back(std::static_pointer_cast<CombatUnit>(p));
    enemies = enemy_units;
  } else {
    std::vector<std::shared_ptr<CombatUnit>> player_units;
    for (const auto& p : players_) player_units.push_back(std::static_pointer_cast<CombatUnit>(p));
    target = CombatUtilities::GetTarget(player_units);
    for (const auto& e : enemies_) friendlies.push_back(std::static_pointer_cast<CombatUnit>(e));
    enemies = player_units;
  }
  bool used_ability = false;
  std::vector<std::shared_ptr<Ability>>* abilities = nullptr;
  if (source->is_player()) {
    abilities = &static_cast<Player*>(source.get())->abilities();
  } else {
    abilities = &static_cast<Monster*>(source.get())->abilities();
  }
  for (const auto& ability : *abilities) {
    if (ability && !used_ability /* && ability->ShouldTrigger(...) && CanUseAbility(...) */) {
      used_ability = TryUseAbility(source, ability);
    }
  }
  if (used_ability) return;
  if (!source->is_blinded()) {
    event_queue_->AddEvent(new events::AutoAttackEvent(simulation_time_ + source->combat_details().combat_stats.attack_interval, source.get()));
  }
}

bool CombatSimulator::CheckEncounterEnd() {
  bool encounter_ended = false;
  // 敌人全灭
  if (!enemies_.empty() &&
      std::none_of(enemies_.begin(), enemies_.end(), [](const std::shared_ptr<Monster>& enemy) {
        return enemy->combat_details().current_hitpoints > 0;
      })) {
    event_queue_->ClearEventsOfType(events::AutoAttackEvent::kType);
    event_queue_->ClearEventsOfType(events::AbilityCastEndEvent::kType);
    event_queue_->AddEvent(new events::EnemyRespawnEvent(simulation_time_ + kEnemyRespawnInterval));
    enemies_.clear();
    // 地牢统计、遭遇统计等可补充
    sim_result_->AddEncounterEnd();
    encounter_ended = true;
  }
  // 玩家死亡检测与复活事件
  for (const auto& player : players_) {
    if (player->combat_details().current_hitpoints <= 0 &&
        !event_queue_->ContainsEventOfTypeAndHrid(events::PlayerRespawnEvent::kType, player->hrid())) {
      event_queue_->AddEvent(new events::PlayerRespawnEvent(simulation_time_ + kPlayerRespawnInterval, player->hrid()));
      sim_result_->AddDeath(player->hrid());
    }
  }
  // 所有玩家死亡
  if (std::none_of(players_.begin(), players_.end(), [](const std::shared_ptr<Player>& p) {
        return p->combat_details().current_hitpoints > 0;
      })) {
    // 地牢失败统计等可补充
    encounter_ended = true;
    all_players_dead_ = true;
  }
  return encounter_ended;
}

bool CombatSimulator::CanUseAbility(const std::shared_ptr<CombatUnit>& source, const std::shared_ptr<Ability>& ability, bool check_oom) {
  // 检查冷却、法力、沉默等，仿照JS canUseAbility
  if (!source || !ability) return false;
  if (source->combat_details().current_hitpoints <= 0) return false;
  if (check_oom && source->combat_details().current_manapoints < ability->mana_cost()) return false;
  if (ability->last_used() + ability->cooldown_duration() > simulation_time_) return false;
  if (source->is_silenced() && ability->mana_cost() > 0) return false;
  return true;
}

bool CombatSimulator::TryUseAbility(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability) {
  if (!CanUseAbility(source, ability, true)) return false;
  // 扣除法力
  source->mutable_combat_details().current_manapoints -= ability->mana_cost();
  // 经验统计
  sim_result_->AddExperienceGain(source->hrid(), "intelligence", CombatUtilities::CalculateIntelligenceExperience(ability->mana_cost()));
  ability->set_last_used(simulation_time_);
  // 冷却
  double haste = source->combat_details().combat_stats.ability_haste;
  double cooldown = ability->cooldown_duration();
  if (haste > 0) cooldown = cooldown * 100 / (100 + haste);
  // 立即安排下一次攻击
  AddNextAttackEvent(source);
  // 技能效果分发
  for (const auto& effect : ability->ability_effects()) {
    const std::string& type = effect.effect_type;
    if (type == "/ability_effect_types/buff") {
      ProcessAbilityBuffEffect(source, ability, effect);
    } else if (type == "/ability_effect_types/damage") {
      ProcessAbilityDamageEffect(source, ability, effect);
    } else if (type == "/ability_effect_types/heal") {
      ProcessAbilityHealEffect(source, ability, effect);
    } else if (type == "/ability_effect_types/revive") {
      ProcessAbilityReviveEffect(source, ability, effect);
    } else if (type == "/ability_effect_types/promote") {
      ProcessAbilityPromoteEffect(source, ability, effect);
    } else if (type == "/ability_effect_types/spend_hp") {
      ProcessAbilitySpendHpEffect(source, ability, effect);
    }
  }
  // 死亡检测
  if (source->combat_details().current_hitpoints == 0) {
    event_queue_->ClearEventsForUnit(static_cast<void*>(source.get()));
    sim_result_->AddDeath(source->hrid());
  }
  CheckEncounterEnd();
  return true;
}

bool CombatSimulator::TryUseConsumable(std::shared_ptr<CombatUnit> source, std::shared_ptr<Consumable> consumable) {
  if (source->combat_details().current_hitpoints <= 0) return false;
  consumable->set_last_used(simulation_time_);
  double cooldown = consumable->cooldown_duration();
  event_queue_->AddEvent(new events::CooldownReadyEvent(simulation_time_ + cooldown));
  sim_result_->AddConsumableUse(source->hrid(), consumable->hrid());
  if (consumable->recovery_duration() == 0) {
    if (consumable->hitpoint_restore() > 0) {
      int added = static_cast<int>(source->AddHitpoints(consumable->hitpoint_restore()));
      sim_result_->AddHitpointsGained(source->hrid(), consumable->hrid(), added);
    }
    if (consumable->manapoint_restore() > 0) {
      int added = static_cast<int>(source->AddManapoints(consumable->manapoint_restore()));
      sim_result_->AddManapointsGained(source->hrid(), consumable->hrid(), added);
    }
  } else {
    int total_ticks = static_cast<int>(consumable->recovery_duration() / kHotTickInterval);
    event_queue_->AddEvent(new events::ConsumableTickEvent(simulation_time_ + kHotTickInterval, source.get(), consumable.get(), total_ticks, 1));
  }
  for (const auto& buff : consumable->buffs()) {
    source->AddBuff(buff, simulation_time_);
    event_queue_->AddEvent(new events::CheckBuffExpirationEvent(simulation_time_ + buff.duration(), source.get()));
  }
  return true;
}

void CombatSimulator::ProcessConsumableTickEvent(events::ConsumableTickEvent* event) {
  auto source = static_cast<CombatUnit*>(event->source());
  auto consumable = static_cast<Consumable*>(event->consumable());
  if (consumable->hitpoint_restore() > 0) {
    int tick_value = CombatUtilities::CalculateTickValue(consumable->hitpoint_restore(), event->total_ticks(), event->current_tick());
    int added = static_cast<int>(source->AddHitpoints(tick_value));
    sim_result_->AddHitpointsGained(source->hrid(), consumable->hrid(), added);
  }
  if (consumable->manapoint_restore() > 0) {
    int tick_value = CombatUtilities::CalculateTickValue(consumable->manapoint_restore(), event->total_ticks(), event->current_tick());
    int added = static_cast<int>(source->AddManapoints(tick_value));
    sim_result_->AddManapointsGained(source->hrid(), consumable->hrid(), added);
  }
  if (event->current_tick() < event->total_ticks()) {
    event_queue_->AddEvent(new events::ConsumableTickEvent(simulation_time_ + kHotTickInterval, source, consumable, event->total_ticks(), event->current_tick() + 1));
  }
}

void CombatSimulator::ProcessDamageOverTimeTickEvent(events::DamageOverTimeEvent* event) {
  auto target = static_cast<CombatUnit*>(event->target());
  int tick_damage = CombatUtilities::CalculateTickValue(event->damage(), event->total_ticks(), event->current_tick());
  int damage = std::min(tick_damage, static_cast<int>(target->combat_details().current_hitpoints));
  target->AddHitpoints(-damage);
  sim_result_->AddAttack(static_cast<CombatUnit*>(event->source())->hrid(), target->hrid(), "damageOverTime", std::to_string(damage));
  sim_result_->AddExperienceGain(target->hrid(), "stamina", CombatUtilities::CalculateStaminaExperience(0, damage));
  if (event->current_tick() < event->total_ticks()) {
    event_queue_->AddEvent(new events::DamageOverTimeEvent(simulation_time_ + kDotTickInterval, event->source(), target, event->damage(), event->total_ticks(), event->current_tick() + 1, event->combat_style_hrid()));
  }
  if (target->combat_details().current_hitpoints == 0) {
    event_queue_->ClearEventsForUnit(static_cast<void*>(target));
    sim_result_->AddDeath(target->hrid());
  }
  CheckEncounterEnd();
}

void CombatSimulator::ProcessAbilityBuffEffect(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability, const AbilityEffect& effect) {
  if (effect.target_type == "allAllies") {
    std::vector<std::shared_ptr<CombatUnit>> targets = source->is_player() ? std::vector<std::shared_ptr<CombatUnit>>(players_.begin(), players_.end()) : std::vector<std::shared_ptr<CombatUnit>>(enemies_.begin(), enemies_.end());
    for (const auto& target : targets) {
      if (target->combat_details().current_hitpoints > 0) {
        for (const auto& buff : effect.buffs) {
          target->AddBuff(buff, simulation_time_);
          event_queue_->AddEvent(new events::CheckBuffExpirationEvent(simulation_time_ + buff.duration(), target.get()));
        }
      }
    }
  } else if (effect.target_type == "self") {
    for (const auto& buff : effect.buffs) {
      source->AddBuff(buff, simulation_time_);
      event_queue_->AddEvent(new events::CheckBuffExpirationEvent(simulation_time_ + buff.duration(), source.get()));
    }
  }
}

// stub: 处理伤害效果
void CombatSimulator::ProcessAbilityDamageEffect(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability, const AbilityEffect& effect) {
  // 仅支持enemy/allEnemies
  std::vector<std::shared_ptr<CombatUnit>> targets = source->is_player() ? std::vector<std::shared_ptr<CombatUnit>>(enemies_.begin(), enemies_.end()) : std::vector<std::shared_ptr<CombatUnit>>(players_.begin(), players_.end());
  for (const auto& target : targets) {
    if (target->combat_details().current_hitpoints <= 0) continue;
    auto attack_result = CombatUtilities::ProcessAttack(*source, *target, &effect);
    sim_result_->AddAttack(source->hrid(), target->hrid(), ability->hrid(), attack_result.did_hit ? std::to_string(attack_result.damage_done) : "miss");
    for (const auto& kv : attack_result.experience_gained_source) {
      sim_result_->AddExperienceGain(source->hrid(), kv.first, kv.second);
    }
    for (const auto& kv : attack_result.experience_gained_target) {
      sim_result_->AddExperienceGain(target->hrid(), kv.first, kv.second);
    }
    if (attack_result.hp_drain > 0) {
      sim_result_->AddHitpointsGained(source->hrid(), ability->hrid(), attack_result.hp_drain);
    }
    if (attack_result.did_hit && !effect.buffs.empty()) {
      for (const auto& buff : effect.buffs) {
        target->AddBuff(buff, simulation_time_);
        event_queue_->AddEvent(new events::CheckBuffExpirationEvent(simulation_time_ + buff.duration(), target.get()));
      }
    }
    if (effect.damage_over_time_ratio > 0 && attack_result.damage_done > 0) {
      int total_ticks = static_cast<int>(effect.damage_over_time_duration / kDotTickInterval);
      event_queue_->AddEvent(new events::DamageOverTimeEvent(simulation_time_ + kDotTickInterval, source.get(), target.get(), attack_result.damage_done * effect.damage_over_time_ratio, total_ticks, 1, effect.combat_style_hrid));
    }
    if (target->combat_details().current_hitpoints == 0) {
      event_queue_->ClearEventsForUnit(static_cast<void*>(target.get()));
      sim_result_->AddDeath(target->hrid());
    }
  }
}

// stub: 处理治疗效果
void CombatSimulator::ProcessAbilityHealEffect(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability, const AbilityEffect& effect) {
  // 支持self/allAllies/lowestHpAlly
  if (effect.target_type == "allAllies") {
    std::vector<std::shared_ptr<CombatUnit>> targets = source->is_player() ? std::vector<std::shared_ptr<CombatUnit>>(players_.begin(), players_.end()) : std::vector<std::shared_ptr<CombatUnit>>(enemies_.begin(), enemies_.end());
    for (const auto& target : targets) {
      if (target->combat_details().current_hitpoints > 0) {
        int amount_healed = CombatUtilities::ProcessHeal(*source, effect, *target);
        sim_result_->AddHitpointsGained(target->hrid(), ability->hrid(), amount_healed);
        sim_result_->AddExperienceGain(source->hrid(), "magic", CombatUtilities::CalculateHealingExperience(amount_healed));
      }
    }
  } else if (effect.target_type == "lowestHpAlly") {
    std::vector<std::shared_ptr<CombatUnit>> targets = source->is_player() ? std::vector<std::shared_ptr<CombatUnit>>(players_.begin(), players_.end()) : std::vector<std::shared_ptr<CombatUnit>>(enemies_.begin(), enemies_.end());
    std::shared_ptr<CombatUnit> heal_target;
    for (const auto& target : targets) {
      if (target->combat_details().current_hitpoints > 0) {
        if (!heal_target || target->combat_details().current_hitpoints < heal_target->combat_details().current_hitpoints) {
          heal_target = target;
        }
      }
    }
    if (heal_target) {
      int amount_healed = CombatUtilities::ProcessHeal(*source, effect, *heal_target);
      sim_result_->AddHitpointsGained(heal_target->hrid(), ability->hrid(), amount_healed);
      sim_result_->AddExperienceGain(source->hrid(), "magic", CombatUtilities::CalculateHealingExperience(amount_healed));
    }
  } else if (effect.target_type == "self") {
    int amount_healed = CombatUtilities::ProcessHeal(*source, effect, *source);
    sim_result_->AddHitpointsGained(source->hrid(), ability->hrid(), amount_healed);
    sim_result_->AddExperienceGain(source->hrid(), "magic", CombatUtilities::CalculateHealingExperience(amount_healed));
  }
}

// stub: 处理复活效果
void CombatSimulator::ProcessAbilityReviveEffect(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability, const AbilityEffect& effect) {
  // 仅支持deadAlly
  std::vector<std::shared_ptr<CombatUnit>> targets = source->is_player() ? std::vector<std::shared_ptr<CombatUnit>>(players_.begin(), players_.end()) : std::vector<std::shared_ptr<CombatUnit>>(enemies_.begin(), enemies_.end());
  for (const auto& target : targets) {
    if (target->combat_details().current_hitpoints <= 0) {
      event_queue_->ClearEventsOfType(events::PlayerRespawnEvent::kType); // 清理复活事件
      int amount_healed = CombatUtilities::ProcessRevive(*source, effect, *target);
      sim_result_->AddHitpointsGained(target->hrid(), ability->hrid(), amount_healed);
      sim_result_->AddExperienceGain(source->hrid(), "magic", CombatUtilities::CalculateHealingExperience(amount_healed));
      AddNextAttackEvent(target);
      break;
    }
  }
}

// stub: 处理晋升效果
void CombatSimulator::ProcessAbilityPromoteEffect(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability, const AbilityEffect& effect) {
  // 仅支持self晋升为特殊怪物，略
}
void CombatSimulator::ProcessCombatStartEvent(events::CombatStartEvent* event) {
  // 重置所有玩家
  for (const auto& player : players_) {
    player->Reset(simulation_time_);
  }
  // 添加第一个RegenTickEvent
  event_queue_->AddEvent(new events::RegenTickEvent(simulation_time_ + kRegenTickInterval));
  // 开始新遭遇
  StartNewEncounter();
}

void CombatSimulator::ProcessPlayerRespawnEvent(events::PlayerRespawnEvent* event) {
  // 找到复活的玩家
  auto it = std::find_if(players_.begin(), players_.end(), [&](const std::shared_ptr<Player>& p) {
    return p->hrid() == event->hrid();
  });
  if (it == players_.end()) return;
  auto& respawning_player = *it;
  // 恢复HP/MP到最大
  double hp_to_full = respawning_player->combat_details().max_hitpoints - respawning_player->combat_details().current_hitpoints;
  respawning_player->AddHitpoints(hp_to_full);
  double mp_to_full = respawning_player->combat_details().max_manapoints - respawning_player->combat_details().current_manapoints;
  respawning_player->AddManapoints(mp_to_full);
  respawning_player->ClearBuffs();
  respawning_player->ClearCCs();
  if (all_players_dead_) {
    all_players_dead_ = false;
    StartAttacks();
  } else {
    AddNextAttackEvent(respawning_player);
  }
}

void CombatSimulator::ProcessEnemyRespawnEvent(events::EnemyRespawnEvent* event) {}
void CombatSimulator::ProcessAutoAttackEvent(events::AutoAttackEvent* event) {
  auto source = static_cast<CombatUnit*>(event->source());
  if (!source) return;
  
  std::vector<std::shared_ptr<CombatUnit>> targets;
  if (source->is_player()) {
    for (const auto& e : enemies_) targets.push_back(std::static_pointer_cast<CombatUnit>(e));
  } else {
    for (const auto& p : players_) targets.push_back(std::static_pointer_cast<CombatUnit>(p));
  }
  if (targets.empty()) return;
  std::vector<std::shared_ptr<CombatUnit>> alive_targets;
  for (const auto& t : targets) {
    if (t && t->combat_details().current_hitpoints > 0) alive_targets.push_back(t);
  }
  for (size_t i = 0; i < alive_targets.size(); ++i) {
    auto target = alive_targets[i];
    auto attack_result = combat_simulator::CombatUtilities::ProcessAttack(*source, *target);
    // 经验、吸血等略
    if (target->combat_details().current_hitpoints == 0) {
      // 死亡处理略
    }
    if (source->combat_details().current_hitpoints == 0 && attack_result.reflect_damage_done != 0) {
      // 反伤致死处理略
    }
  }
  
  // 查找对应的shared_ptr来继续下一次攻击
  std::shared_ptr<CombatUnit> source_shared;
  if (source->is_player()) {
    for (const auto& p : players_) {
      if (p.get() == source) {
        source_shared = std::static_pointer_cast<CombatUnit>(p);
        break;
      }
    }
  } else {
    for (const auto& e : enemies_) {
      if (e.get() == source) {
        source_shared = std::static_pointer_cast<CombatUnit>(e);
        break;
      }
    }
  }
  
  if (source_shared && !CheckEncounterEnd()) {
    AddNextAttackEvent(source_shared);
  }
}
void CombatSimulator::ProcessConsumableTickEvent(events::ConsumableTickEvent* event) {
  auto source = static_cast<CombatUnit*>(event->source());
  auto consumable = static_cast<Consumable*>(event->consumable());
  if (consumable->hitpoint_restore() > 0) {
    int tick_value = CombatUtilities::CalculateTickValue(consumable->hitpoint_restore(), event->total_ticks(), event->current_tick());
    int added = static_cast<int>(source->AddHitpoints(tick_value));
    sim_result_->AddHitpointsGained(source->hrid(), consumable->hrid(), added);
  }
  if (consumable->manapoint_restore() > 0) {
    int tick_value = CombatUtilities::CalculateTickValue(consumable->manapoint_restore(), event->total_ticks(), event->current_tick());
    int added = static_cast<int>(source->AddManapoints(tick_value));
    sim_result_->AddManapointsGained(source->hrid(), consumable->hrid(), added);
  }
  if (event->current_tick() < event->total_ticks()) {
    event_queue_->AddEvent(new events::ConsumableTickEvent(simulation_time_ + kHotTickInterval, source, consumable, event->total_ticks(), event->current_tick() + 1));
  }
}

void CombatSimulator::ProcessDamageOverTimeTickEvent(events::DamageOverTimeEvent* event) {
  auto target = static_cast<CombatUnit*>(event->target());
  int tick_damage = CombatUtilities::CalculateTickValue(event->damage(), event->total_ticks(), event->current_tick());
  int damage = std::min(tick_damage, static_cast<int>(target->combat_details().current_hitpoints));
  target->AddHitpoints(-damage);
  sim_result_->AddAttack(static_cast<CombatUnit*>(event->source())->hrid(), target->hrid(), "damageOverTime", std::to_string(damage));
  sim_result_->AddExperienceGain(target->hrid(), "stamina", CombatUtilities::CalculateStaminaExperience(0, damage));
  if (event->current_tick() < event->total_ticks()) {
    event_queue_->AddEvent(new events::DamageOverTimeEvent(simulation_time_ + kDotTickInterval, event->source(), target, event->damage(), event->total_ticks(), event->current_tick() + 1, event->combat_style_hrid()));
  }
  if (target->combat_details().current_hitpoints == 0) {
    event_queue_->ClearEventsForUnit(static_cast<void*>(target));
    sim_result_->AddDeath(target->hrid());
  }
  CheckEncounterEnd();
}

void CombatSimulator::ProcessAbilityBuffEffect(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability, const AbilityEffect& effect) {
  if (effect.target_type == "allAllies") {
    std::vector<std::shared_ptr<CombatUnit>> targets = source->is_player() ? std::vector<std::shared_ptr<CombatUnit>>(players_.begin(), players_.end()) : std::vector<std::shared_ptr<CombatUnit>>(enemies_.begin(), enemies_.end());
    for (const auto& target : targets) {
      if (target->combat_details().current_hitpoints > 0) {
        for (const auto& buff : effect.buffs) {
          target->AddBuff(buff, simulation_time_);
          event_queue_->AddEvent(new events::CheckBuffExpirationEvent(simulation_time_ + buff.duration(), target.get()));
        }
      }
    }
  } else if (effect.target_type == "self") {
    for (const auto& buff : effect.buffs) {
      source->AddBuff(buff, simulation_time_);
      event_queue_->AddEvent(new events::CheckBuffExpirationEvent(simulation_time_ + buff.duration(), source.get()));
    }
  }
}

// stub: 处理伤害效果
void CombatSimulator::ProcessAbilityDamageEffect(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability, const AbilityEffect& effect) {
  // 仅支持enemy/allEnemies
  std::vector<std::shared_ptr<CombatUnit>> targets = source->is_player() ? std::vector<std::shared_ptr<CombatUnit>>(enemies_.begin(), enemies_.end()) : std::vector<std::shared_ptr<CombatUnit>>(players_.begin(), players_.end());
  for (const auto& target : targets) {
    if (target->combat_details().current_hitpoints <= 0) continue;
    auto attack_result = CombatUtilities::ProcessAttack(*source, *target, &effect);
    sim_result_->AddAttack(source->hrid(), target->hrid(), ability->hrid(), attack_result.did_hit ? std::to_string(attack_result.damage_done) : "miss");
    for (const auto& kv : attack_result.experience_gained_source) {
      sim_result_->AddExperienceGain(source->hrid(), kv.first, kv.second);
    }
    for (const auto& kv : attack_result.experience_gained_target) {
      sim_result_->AddExperienceGain(target->hrid(), kv.first, kv.second);
    }
    if (attack_result.hp_drain > 0) {
      sim_result_->AddHitpointsGained(source->hrid(), ability->hrid(), attack_result.hp_drain);
    }
    if (attack_result.did_hit && !effect.buffs.empty()) {
      for (const auto& buff : effect.buffs) {
        target->AddBuff(buff, simulation_time_);
        event_queue_->AddEvent(new events::CheckBuffExpirationEvent(simulation_time_ + buff.duration(), target.get()));
      }
    }
    if (effect.damage_over_time_ratio > 0 && attack_result.damage_done > 0) {
      int total_ticks = static_cast<int>(effect.damage_over_time_duration / kDotTickInterval);
      event_queue_->AddEvent(new events::DamageOverTimeEvent(simulation_time_ + kDotTickInterval, source.get(), target.get(), attack_result.damage_done * effect.damage_over_time_ratio, total_ticks, 1, effect.combat_style_hrid));
    }
    if (target->combat_details().current_hitpoints == 0) {
      event_queue_->ClearEventsForUnit(static_cast<void*>(target.get()));
      sim_result_->AddDeath(target->hrid());
    }
  }
}

// stub: 处理治疗效果
void CombatSimulator::ProcessAbilityHealEffect(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability, const AbilityEffect& effect) {
  // 支持self/allAllies/lowestHpAlly
  if (effect.target_type == "allAllies") {
    std::vector<std::shared_ptr<CombatUnit>> targets = source->is_player() ? std::vector<std::shared_ptr<CombatUnit>>(players_.begin(), players_.end()) : std::vector<std::shared_ptr<CombatUnit>>(enemies_.begin(), enemies_.end());
    for (const auto& target : targets) {
      if (target->combat_details().current_hitpoints > 0) {
        int amount_healed = CombatUtilities::ProcessHeal(*source, effect, *target);
        sim_result_->AddHitpointsGained(target->hrid(), ability->hrid(), amount_healed);
        sim_result_->AddExperienceGain(source->hrid(), "magic", CombatUtilities::CalculateHealingExperience(amount_healed));
      }
    }
  } else if (effect.target_type == "lowestHpAlly") {
    std::vector<std::shared_ptr<CombatUnit>> targets = source->is_player() ? std::vector<std::shared_ptr<CombatUnit>>(players_.begin(), players_.end()) : std::vector<std::shared_ptr<CombatUnit>>(enemies_.begin(), enemies_.end());
    std::shared_ptr<CombatUnit> heal_target;
    for (const auto& target : targets) {
      if (target->combat_details().current_hitpoints > 0) {
        if (!heal_target || target->combat_details().current_hitpoints < heal_target->combat_details().current_hitpoints) {
          heal_target = target;
        }
      }
    }
    if (heal_target) {
      int amount_healed = CombatUtilities::ProcessHeal(*source, effect, *heal_target);
      sim_result_->AddHitpointsGained(heal_target->hrid(), ability->hrid(), amount_healed);
      sim_result_->AddExperienceGain(source->hrid(), "magic", CombatUtilities::CalculateHealingExperience(amount_healed));
    }
  } else if (effect.target_type == "self") {
    int amount_healed = CombatUtilities::ProcessHeal(*source, effect, *source);
    sim_result_->AddHitpointsGained(source->hrid(), ability->hrid(), amount_healed);
    sim_result_->AddExperienceGain(source->hrid(), "magic", CombatUtilities::CalculateHealingExperience(amount_healed));
  }
}

// stub: 处理复活效果
void CombatSimulator::ProcessAbilityReviveEffect(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability, const AbilityEffect& effect) {
  // 仅支持deadAlly
  std::vector<std::shared_ptr<CombatUnit>> targets = source->is_player() ? std::vector<std::shared_ptr<CombatUnit>>(players_.begin(), players_.end()) : std::vector<std::shared_ptr<CombatUnit>>(enemies_.begin(), enemies_.end());
  for (const auto& target : targets) {
    if (target->combat_details().current_hitpoints <= 0) {
      event_queue_->ClearEventsOfType(events::PlayerRespawnEvent::kType); // 清理复活事件
      int amount_healed = CombatUtilities::ProcessRevive(*source, effect, *target);
      sim_result_->AddHitpointsGained(target->hrid(), ability->hrid(), amount_healed);
      sim_result_->AddExperienceGain(source->hrid(), "magic", CombatUtilities::CalculateHealingExperience(amount_healed));
      AddNextAttackEvent(target);
      break;
    }
  }
}

// stub: 处理晋升效果
void CombatSimulator::ProcessAbilityPromoteEffect(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability, const AbilityEffect& effect) {
  // 仅支持self晋升为特殊怪物，略
}

void CombatSimulator::ProcessCombatStartEvent(events::CombatStartEvent* event) {
  // 重置所有玩家
  for (const auto& player : players_) {
    player->Reset(simulation_time_);
  }
  // 添加第一个RegenTickEvent
  event_queue_->AddEvent(new events::RegenTickEvent(simulation_time_ + kRegenTickInterval));
  // 开始新遭遇
  StartNewEncounter();
}

void CombatSimulator::ProcessPlayerRespawnEvent(events::PlayerRespawnEvent* event) {
  // 找到复活的玩家
  auto it = std::find_if(players_.begin(), players_.end(), [&](const std::shared_ptr<Player>& p) {
    return p->hrid() == event->hrid();
  });
  if (it == players_.end()) return;
  auto& respawning_player = *it;
  // 恢复HP/MP到最大
  double hp_to_full = respawning_player->combat_details().max_hitpoints - respawning_player->combat_details().current_hitpoints;
  respawning_player->AddHitpoints(hp_to_full);
  double mp_to_full = respawning_player->combat_details().max_manapoints - respawning_player->combat_details().current_manapoints;
  respawning_player->AddManapoints(mp_to_full);
  respawning_player->ClearBuffs();
  respawning_player->ClearCCs();
  if (all_players_dead_) {
    all_players_dead_ = false;
    StartAttacks();
  } else {
    AddNextAttackEvent(respawning_player);
  }
}

void CombatSimulator::ProcessEnemyRespawnEvent(events::EnemyRespawnEvent* event) {}
void CombatSimulator::ProcessAutoAttackEvent(events::AutoAttackEvent* event) {
  auto source = static_cast<CombatUnit*>(event->source());
  if (!source) return;
  
  std::vector<std::shared_ptr<CombatUnit>> targets;
  if (source->is_player()) {
    for (const auto& e : enemies_) targets.push_back(std::static_pointer_cast<CombatUnit>(e));
  } else {
    for (const auto& p : players_) targets.push_back(std::static_pointer_cast<CombatUnit>(p));
  }
  if (targets.empty()) return;
  std::vector<std::shared_ptr<CombatUnit>> alive_targets;
  for (const auto& t : targets) {
    if (t && t->combat_details().current_hitpoints > 0) alive_targets.push_back(t);
  }
  for (size_t i = 0; i < alive_targets.size(); ++i) {
    auto target = alive_targets[i];
    auto attack_result = combat_simulator::CombatUtilities::ProcessAttack(*source, *target);
    // 经验、吸血等略
    if (target->combat_details().current_hitpoints == 0) {
      // 死亡处理略
    }
    if (source->combat_details().current_hitpoints == 0 && attack_result.reflect_damage_done != 0) {
      // 反伤致死处理略
    }
  }
  
  // 查找对应的shared_ptr来继续下一次攻击
  std::shared_ptr<CombatUnit> source_shared;
  if (source->is_player()) {
    for (const auto& p : players_) {
      if (p.get() == source) {
        source_shared = std::static_pointer_cast<CombatUnit>(p);
        break;
      }
    }
  } else {
    for (const auto& e : enemies_) {
      if (e.get() == source) {
        source_shared = std::static_pointer_cast<CombatUnit>(e);
        break;
      }
    }
  }
  
  if (source_shared && !CheckEncounterEnd()) {
    AddNextAttackEvent(source_shared);
  }
}
void CombatSimulator::ProcessConsumableTickEvent(events::ConsumableTickEvent* event) {
  auto source = static_cast<CombatUnit*>(event->source());
  auto consumable = static_cast<Consumable*>(event->consumable());
  if (consumable->hitpoint_restore() > 0) {
    int tick_value = CombatUtilities::CalculateTickValue(consumable->hitpoint_restore(), event->total_ticks(), event->current_tick());
    int added = static_cast<int>(source->AddHitpoints(tick_value));
    sim_result_->AddHitpointsGained(source->hrid(), consumable->hrid(), added);
  }
  if (consumable->manapoint_restore() > 0) {
    int tick_value = CombatUtilities::CalculateTickValue(consumable->manapoint_restore(), event->total_ticks(), event->current_tick());
    int added = static_cast<int>(source->AddManapoints(tick_value));
    sim_result_->AddManapointsGained(source->hrid(), consumable->hrid(), added);
  }
  if (event->current_tick() < event->total_ticks()) {
    event_queue_->AddEvent(new events::ConsumableTickEvent(simulation_time_ + kHotTickInterval, source, consumable, event->total_ticks(), event->current_tick() + 1));
  }
}

void CombatSimulator::ProcessDamageOverTimeTickEvent(events::DamageOverTimeEvent* event) {
  auto target = static_cast<CombatUnit*>(event->target());
  int tick_damage = CombatUtilities::CalculateTickValue(event->damage(), event->total_ticks(), event->current_tick());
  int damage = std::min(tick_damage, static_cast<int>(target->combat_details().current_hitpoints));
  target->AddHitpoints(-damage);
  sim_result_->AddAttack(static_cast<CombatUnit*>(event->source())->hrid(), target->hrid(), "damageOverTime", std::to_string(damage));
  sim_result_->AddExperienceGain(target->hrid(), "stamina", CombatUtilities::CalculateStaminaExperience(0, damage));
  if (event->current_tick() < event->total_ticks()) {
    event_queue_->AddEvent(new events::DamageOverTimeEvent(simulation_time_ + kDotTickInterval, event->source(), target, event->damage(), event->total_ticks(), event->current_tick() + 1, event->combat_style_hrid()));
  }
  if (target->combat_details().current_hitpoints == 0) {
    event_queue_->ClearEventsForUnit(static_cast<void*>(target));
    sim_result_->AddDeath(target->hrid());
  }
  CheckEncounterEnd();
}

void CombatSimulator::ProcessAbilityBuffEffect(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability, const AbilityEffect& effect) {
  if (effect.target_type == "allAllies") {
    std::vector<std::shared_ptr<CombatUnit>> targets = source->is_player() ? std::vector<std::shared_ptr<CombatUnit>>(players_.begin(), players_.end()) : std::vector<std::shared_ptr<CombatUnit>>(enemies_.begin(), enemies_.end());
    for (const auto& target : targets) {
      if (target->combat_details().current_hitpoints > 0) {
        for (const auto& buff : effect.buffs) {
          target->AddBuff(buff, simulation_time_);
          event_queue_->AddEvent(new events::CheckBuffExpirationEvent(simulation_time_ + buff.duration(), target.get()));
        }
      }
    }
  } else if (effect.target_type == "self") {
    for (const auto& buff : effect.buffs) {
      source->AddBuff(buff, simulation_time_);
      event_queue_->AddEvent(new events::CheckBuffExpirationEvent(simulation_time_ + buff.duration(), source.get()));
    }
  }
}

// stub: 处理伤害效果
void CombatSimulator::ProcessAbilityDamageEffect(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability, const AbilityEffect& effect) {
  // 仅支持enemy/allEnemies
  std::vector<std::shared_ptr<CombatUnit>> targets = source->is_player() ? std::vector<std::shared_ptr<CombatUnit>>(enemies_.begin(), enemies_.end()) : std::vector<std::shared_ptr<CombatUnit>>(players_.begin(), players_.end());
  for (const auto& target : targets) {
    if (target->combat_details().current_hitpoints <= 0) continue;
    auto attack_result = CombatUtilities::ProcessAttack(*source, *target, &effect);
    sim_result_->AddAttack(source->hrid(), target->hrid(), ability->hrid(), attack_result.did_hit ? std::to_string(attack_result.damage_done) : "miss");
    for (const auto& kv : attack_result.experience_gained_source) {
      sim_result_->AddExperienceGain(source->hrid(), kv.first, kv.second);
    }
    for (const auto& kv : attack_result.experience_gained_target) {
      sim_result_->AddExperienceGain(target->hrid(), kv.first, kv.second);
    }
    if (attack_result.hp_drain > 0) {
      sim_result_->AddHitpointsGained(source->hrid(), ability->hrid(), attack_result.hp_drain);
    }
    if (attack_result.did_hit && !effect.buffs.empty()) {
      for (const auto& buff : effect.buffs) {
        target->AddBuff(buff, simulation_time_);
        event_queue_->AddEvent(new events::CheckBuffExpirationEvent(simulation_time_ + buff.duration(), target.get()));
      }
    }
    if (effect.damage_over_time_ratio > 0 && attack_result.damage_done > 0) {
      int total_ticks = static_cast<int>(effect.damage_over_time_duration / kDotTickInterval);
      event_queue_->AddEvent(new events::DamageOverTimeEvent(simulation_time_ + kDotTickInterval, source.get(), target.get(), attack_result.damage_done * effect.damage_over_time_ratio, total_ticks, 1, effect.combat_style_hrid));
    }
    if (target->combat_details().current_hitpoints == 0) {
      event_queue_->ClearEventsForUnit(static_cast<void*>(target.get()));
      sim_result_->AddDeath(target->hrid());
    }
  }
}

// stub: 处理治疗效果
void CombatSimulator::ProcessAbilityHealEffect(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability, const AbilityEffect& effect) {
  // 支持self/allAllies/lowestHpAlly
  if (effect.target_type == "allAllies") {
    std::vector<std::shared_ptr<CombatUnit>> targets = source->is_player() ? std::vector<std::shared_ptr<CombatUnit>>(players_.begin(), players_.end()) : std::vector<std::shared_ptr<CombatUnit>>(enemies_.begin(), enemies_.end());
    for (const auto& target : targets) {
      if (target->combat_details().current_hitpoints > 0) {
        int amount_healed = CombatUtilities::ProcessHeal(*source, effect, *target);
        sim_result_->AddHitpointsGained(target->hrid(), ability->hrid(), amount_healed);
        sim_result_->AddExperienceGain(source->hrid(), "magic", CombatUtilities::CalculateHealingExperience(amount_healed));
      }
    }
  } else if (effect.target_type == "lowestHpAlly") {
    std::vector<std::shared_ptr<CombatUnit>> targets = source->is_player() ? std::vector<std::shared_ptr<CombatUnit>>(players_.begin(), players_.end()) : std::vector<std::shared_ptr<CombatUnit>>(enemies_.begin(), enemies_.end());
    std::shared_ptr<CombatUnit> heal_target;
    for (const auto& target : targets) {
      if (target->combat_details().current_hitpoints > 0) {
        if (!heal_target || target->combat_details().current_hitpoints < heal_target->combat_details().current_hitpoints) {
          heal_target = target;
        }
      }
    }
    if (heal_target) {
      int amount_healed = CombatUtilities::ProcessHeal(*source, effect, *heal_target);
      sim_result_->AddHitpointsGained(heal_target->hrid(), ability->hrid(), amount_healed);
      sim_result_->AddExperienceGain(source->hrid(), "magic", CombatUtilities::CalculateHealingExperience(amount_healed));
    }
  } else if (effect.target_type == "self") {
    int amount_healed = CombatUtilities::ProcessHeal(*source, effect, *source);
    sim_result_->AddHitpointsGained(source->hrid(), ability->hrid(), amount_healed);
    sim_result_->AddExperienceGain(source->hrid(), "magic", CombatUtilities::CalculateHealingExperience(amount_healed));
  }
}

// stub: 处理复活效果
void CombatSimulator::ProcessAbilityReviveEffect(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability, const AbilityEffect& effect) {
  // 仅支持deadAlly
  std::vector<std::shared_ptr<CombatUnit>> targets = source->is_player() ? std::vector<std::shared_ptr<CombatUnit>>(players_.begin(), players_.end()) : std::vector<std::shared_ptr<CombatUnit>>(enemies_.begin(), enemies_.end());
  for (const auto& target : targets) {
    if (target->combat_details().current_hitpoints <= 0) {
      event_queue_->ClearEventsOfType(events::PlayerRespawnEvent::kType); // 清理复活事件
      int amount_healed = CombatUtilities::ProcessRevive(*source, effect, *target);
      sim_result_->AddHitpointsGained(target->hrid(), ability->hrid(), amount_healed);
      sim_result_->AddExperienceGain(source->hrid(), "magic", CombatUtilities::CalculateHealingExperience(amount_healed));
      AddNextAttackEvent(target);
      break;
    }
  }
}

// stub: 处理晋升效果
void CombatSimulator::ProcessAbilityPromoteEffect(std::shared_ptr<CombatUnit> source, std::shared_ptr<Ability> ability, const AbilityEffect& effect) {
  // 仅支持self晋升为特殊怪物，略
}