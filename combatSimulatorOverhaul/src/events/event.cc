// Copyright 2025 BKNMWICombatSimulator
//
// event.cc - Implementation of the base Event class for the combat simulator.

#include "events/event.h"

Event::Event(double time, const std::string& type,
                         const std::string& hrid,
                         void* source,
                         void* target)
    : time_(time),
      type_(type),
      hrid_(hrid),
      source_(source),
      target_(target) {}
