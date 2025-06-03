// Copyright 2025 BKNMWICombatSimulator
//
// combat_events.h - All combat event types for the combat simulator.
// This file merges all event types into a single header for convenience.

#ifndef COMBAT_SIMULATOR_OVERHAUL_INCLUDE_COMBAT_EVENTS_H_
#define COMBAT_SIMULATOR_OVERHAUL_INCLUDE_COMBAT_EVENTS_H_

#include <string>
#include <memory>
#include <algorithm>
#include <cstdint>
#include "events/event.h"

namespace combat_simulator {
namespace events {

// AbilityCastEndEvent
class AbilityCastEndEvent : public Event {
 public:
  static constexpr const char* kType = "abilityCastEndEvent";
  AbilityCastEndEvent(double time, void* source, void* ability)
      : Event(time, kType, "", source), ability_(ability) {}
  void* ability() const { return ability_; }
 private:
  void* ability_;
};

// AutoAttackEvent
class AutoAttackEvent : public Event {
 public:
  static constexpr const char* kType = "autoAttack";
  AutoAttackEvent(double time, void* source)
      : Event(time, kType, "", source) {}
};

// AwaitCooldownEvent
class AwaitCooldownEvent : public Event {
 public:
  static constexpr const char* kType = "awaitCooldownEvent";
  AwaitCooldownEvent(double time, void* source)
      : Event(time, kType, "", source) {}
};

// BlindExpirationEvent
class BlindExpirationEvent : public Event {
 public:
  static constexpr const char* kType = "blindExpiration";
  BlindExpirationEvent(double time, void* source)
      : Event(time, kType, "", source) {}
};

// CheckBuffExpirationEvent
class CheckBuffExpirationEvent : public Event {
 public:
  static constexpr const char* kType = "checkBuffExpiration";
  CheckBuffExpirationEvent(double time, void* source)
      : Event(time, kType, "", source) {}
};

// CombatStartEvent
class CombatStartEvent : public Event {
 public:
  static constexpr const char* kType = "combatStart";
  explicit CombatStartEvent(double time)
      : Event(time, kType) {}
};

// ConsumableTickEvent
class ConsumableTickEvent : public Event {
 public:
  static constexpr const char* kType = "consumableTick";
  ConsumableTickEvent(double time, void* source, void* consumable, int total_ticks, int current_tick)
      : Event(time, kType, "", source), consumable_(consumable), total_ticks_(total_ticks), current_tick_(current_tick) {}
  void* consumable() const { return consumable_; }
  int total_ticks() const { return total_ticks_; }
  int current_tick() const { return current_tick_; }
 private:
  void* consumable_;
  int total_ticks_;
  int current_tick_;
};

// CooldownReadyEvent
class CooldownReadyEvent : public Event {
 public:
  static constexpr const char* kType = "cooldownReady";
  explicit CooldownReadyEvent(double time)
      : Event(time, kType) {}
};

// CurseExpirationEvent
class CurseExpirationEvent : public Event {
 public:
  static constexpr const char* kType = "curseExpiration";
  CurseExpirationEvent(double time, void* source)
      : Event(time, kType, "", source) {}
};

// DamageOverTimeEvent
class DamageOverTimeEvent : public Event {
 public:
  static constexpr const char* kType = "damageOverTime";
  DamageOverTimeEvent(double time, void* source_ref, void* target, double damage, int total_ticks, int current_tick, const std::string& combat_style_hrid)
      : Event(time, kType, "", source_ref, target), damage_(damage), total_ticks_(total_ticks), current_tick_(current_tick), combat_style_hrid_(combat_style_hrid) {}
  double damage() const { return damage_; }
  int total_ticks() const { return total_ticks_; }
  int current_tick() const { return current_tick_; }
  const std::string& combat_style_hrid() const { return combat_style_hrid_; }
 private:
  double damage_;
  int total_ticks_;
  int current_tick_;
  std::string combat_style_hrid_;
};

// EnemyRespawnEvent
class EnemyRespawnEvent : public Event {
 public:
  static constexpr const char* kType = "enemyRespawn";
  explicit EnemyRespawnEvent(double time)
      : Event(time, kType) {}
};

// FuryExpirationEvent
class FuryExpirationEvent : public Event {
 public:
  static constexpr const char* kType = "furyExpiration";
  FuryExpirationEvent(double time, void* source)
      : Event(time, kType, "", source) {}
};

// PlayerRespawnEvent
class PlayerRespawnEvent : public Event {
 public:
  static constexpr const char* kType = "playerRespawn";
  PlayerRespawnEvent(double time, const std::string& hrid)
      : Event(time, kType, hrid) {}
};

// RegenTickEvent
class RegenTickEvent : public Event {
 public:
  static constexpr const char* kType = "regenTick";
  explicit RegenTickEvent(double time)
      : Event(time, kType) {}
};

// StunExpirationEvent
class StunExpirationEvent : public Event {
 public:
  static constexpr const char* kType = "stunExpiration";
  StunExpirationEvent(double time, void* source)
      : Event(time, kType, "", source) {}
};

// SilenceExpirationEvent
class SilenceExpirationEvent : public Event {
 public:
  static constexpr const char* kType = "silenceExpiration";
  SilenceExpirationEvent(double time, void* source)
      : Event(time, kType, "", source) {}
};

// WeakenExpirationEvent
class WeakenExpirationEvent : public Event {
 public:
  static constexpr const char* kType = "weakenExpiration";
  static constexpr int kMaxWeakenStacks = 5;
  WeakenExpirationEvent(double time, int weaken_amount, void* source)
      : Event(time, kType, "", source), weaken_amount_(std::min(weaken_amount + 1, kMaxWeakenStacks)) {}
  int weaken_amount() const { return weaken_amount_; }
 private:
  int weaken_amount_;
};

}  // namespace events
}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_OVERHAUL_INCLUDE_COMBAT_EVENTS_H_
