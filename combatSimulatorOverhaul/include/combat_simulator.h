#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include "EventQueue.h"
#include "SimResult.h"
#include "Player.h"
#include "Zone.h"

// Constants
const long long ONE_SECOND = 1000000000;
const long long HOT_TICK_INTERVAL = 5 * ONE_SECOND;
const long long DOT_TICK_INTERVAL = 3 * ONE_SECOND;
const long long REGEN_TICK_INTERVAL = 10 * ONE_SECOND;
const long long ENEMY_RESPAWN_INTERVAL = 3 * ONE_SECOND;
const long long PLAYER_RESPAWN_INTERVAL = 150 * ONE_SECOND;
const long long RESTART_INTERVAL = 15 * ONE_SECOND;

class CombatSimulator {
public:
    CombatSimulator(std::vector<std::shared_ptr<Player>> players, std::shared_ptr<Zone> zone, const std::string& simId = "");
    ~CombatSimulator();

    // Main simulation function
    std::shared_ptr<SimResult> simulate(long long simulationTimeLimit);

    // Event callbacks
    typedef std::function<void(const std::string&, double)> ProgressCallback;
    void setProgressCallback(ProgressCallback callback);

private:
    // Member variables
    std::vector<std::shared_ptr<Player>> players;
    std::shared_ptr<Zone> zone;
    std::shared_ptr<EventQueue> eventQueue;
    std::string simId;
    std::shared_ptr<SimResult> simResult;
    bool allPlayersDead;
    std::vector<std::shared_ptr<Monster>> enemies;
    long long simulationTime;
    ProgressCallback progressCallback;
    int tempDungeonCount;

    // Helper methods
    void reset();
    void processEvent(std::shared_ptr<Event> event);
    
    void processCombatStartEvent(std::shared_ptr<Event> event);
    void processPlayerRespawnEvent(std::shared_ptr<Event> event);
    void processEnemyRespawnEvent(std::shared_ptr<Event> event);
    void processAutoAttackEvent(std::shared_ptr<Event> event);
    void processConsumableTickEvent(std::shared_ptr<Event> event);
    void processDamageOverTimeTickEvent(std::shared_ptr<Event> event);
    void processRegenTickEvent(std::shared_ptr<Event> event);
    void processCheckBuffExpirationEvent(std::shared_ptr<Event> event);
    void processStunExpirationEvent(std::shared_ptr<Event> event);
    void processBlindExpirationEvent(std::shared_ptr<Event> event);
    void processSilenceExpirationEvent(std::shared_ptr<Event> event);
    void processCurseExpirationEvent(std::shared_ptr<Event> event);
    void processWeakenExpirationEvent(std::shared_ptr<Event> event);
    void processFuryExpirationEvent(std::shared_ptr<Event> event);
    
    void startNewEncounter();
    void startAttacks();
    void addNextAttackEvent(std::shared_ptr<CombatEntity> source);
    bool checkEncounterEnd();
    void checkTriggers();
    bool checkTriggersForUnit(std::shared_ptr<CombatEntity> unit, 
                             const std::vector<std::shared_ptr<CombatEntity>>& friendlies,
                             const std::vector<std::shared_ptr<CombatEntity>>& enemies);
    
    bool tryUseConsumable(std::shared_ptr<CombatEntity> source, std::shared_ptr<Consumable> consumable);
    bool canUseAbility(std::shared_ptr<CombatEntity> source, std::shared_ptr<Ability> ability, bool oomCheck);
    bool tryUseAbility(std::shared_ptr<CombatEntity> source, std::shared_ptr<Ability> ability);
    
    void processAbilityBuffEffect(std::shared_ptr<CombatEntity> source, 
                                 std::shared_ptr<Ability> ability, 
                                 const AbilityEffect& abilityEffect);
    void processAbilityDamageEffect(std::shared_ptr<CombatEntity> source, 
                                   std::shared_ptr<Ability> ability, 
                                   const AbilityEffect& abilityEffect);
    void processAbilityHealEffect(std::shared_ptr<CombatEntity> source, 
                                 std::shared_ptr<Ability> ability, 
                                 const AbilityEffect& abilityEffect);
    void processAbilityReviveEffect(std::shared_ptr<CombatEntity> source, 
                                   std::shared_ptr<Ability> ability, 
                                   const AbilityEffect& abilityEffect);
    std::shared_ptr<Monster> processAbilityPromoteEffect(std::shared_ptr<CombatEntity> source, 
                                                        std::shared_ptr<Ability> ability, 
                                                        const AbilityEffect& abilityEffect);
    void processAbilitySpendHpEffect(std::shared_ptr<CombatEntity> source, 
                                    std::shared_ptr<Ability> ability, 
                                    const AbilityEffect& abilityEffect);
};
