// Copyright 2025 BKNMWICombatSimulator
//
// Implementation of the Buff class for the combat simulator.

#include "buff.h"

namespace combat_simulator {

Buff::Buff(const Buff& buff, int level) {
  unique_hrid_ = buff.unique_hrid_;
  type_hrid_ = buff.type_hrid_;
  ratio_boost_ = buff.ratio_boost_ + (level - 1) * buff.ratio_boost_level_bonus_;
  flat_boost_ = buff.flat_boost_ + (level - 1) * buff.flat_boost_level_bonus_;
  duration_ = buff.duration_;
  ratio_boost_level_bonus_ = buff.ratio_boost_level_bonus_;
  flat_boost_level_bonus_ = buff.flat_boost_level_bonus_;
  start_time_ = buff.start_time_;
}

}  // namespace combat_simulator
