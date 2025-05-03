import Player from "./player";

const ONE_SECOND = 1e9;
const ONE_HOUR = 60 * 60 * ONE_SECOND;

const OPTIMIZABLE_TRIGGERS = {
    // [min, max, step]
    "/combat_trigger_conditions/lowest_hp_percentage": [0, 100, 20],
    "/combat_trigger_conditions/missing_hp": [0, 5000, 500],
    "/combat_trigger_conditions/missing_mp": [0, 5000, 500],
    "/combat_trigger_conditions/current_hp": [0, 5000, 500],
    "/combat_trigger_conditions/current_mp": [0, 5000, 500],
}
const OPTIMIZABLE_TARGETS = {
    "EPH": (simResult) => null, // calculate separately later
    "encounters": (simResult) => simResult.encounters,
    "noRngProfit": (simResult) => simResult.targetValue,
}

async function triggerOptimizer(players, zoneHrid, optimizeTarget){
    const [triggersMap, args] = getPlayersOptimizableTriggersMap(players);
    if (!triggersMap || triggersMap.length == 0) {
        console.log("No optimizable triggers found.");
        return null;
    }
    const result = await adamOptimizer(players, zoneHrid, triggersMap, args, optimizeTarget);
    return result;
}

async function adamOptimizer(players, zoneHrid, triggerMap, args, optimizeTarget="EPH", baseSimulationTime=24, learningRate = 1, beta1 = 0.9, beta2 = 0.999, epsilon = 1e-8, maxIterations = 1000) {
    let m = new Array(args.length).fill(0);
    let v = new Array(args.length).fill(0);
    let t = 0;

    for (let i = 0; i < maxIterations; i++) {
        t++;
        const startTime = new Date().getTime();
        const [gradients, maxValue] = await computeGradients(players, zoneHrid, triggerMap, args, optimizeTarget, baseSimulationTime);
        for (let j = 0; j < args.length; j++) {
            m[j] = beta1 * m[j] + (1 - beta1) * gradients[j];
            v[j] = beta2 * v[j] + (1 - beta2) * gradients[j] ** 2;

            const mHat = m[j] / (1 - beta1 ** t);
            const vHat = v[j] / (1 - beta2 ** t);

            args[j] += learningRate * mHat / (Math.sqrt(vHat) + epsilon);
            // Clamp the value to the min and max range
            const min = triggerMap[j][5];
            const max = triggerMap[j][6];
            args[j] = Math.max(min, Math.min(max, args[j]));
        }
        // const simulationResult = mockSimulation(triggers);
        const useTime = new Date().getTime() - startTime;
        const statString = getDetailFromTriggerMap(players, triggerMap, args).map((trigger) => {
            return `${trigger.player.hrid} ${trigger.item.hrid} ${Math.round(trigger.value)}`;
        }).join("\n    ");
        console.log(`Iteration ${i + 1}: ${optimizeTarget} ${maxValue.toFixed(3)}\n    ${statString}\n(Sim ${args.length*2}*${baseSimulationTime}hr in ${useTime}ms)`);
    }
    return args;
}

async function computeGradients(players, zoneHrid, triggerMap, args, optimizeTarget="EPH", simulationTimeHours = 24) {
    const gradients = [];
    // calculate all value concurrently
    const simulatorWorkerPool = []
    for (let i = 0; i < args.length; i++) {
        const playerIndex = triggerMap[i][0];
        const itemIndex = triggerMap[i][1];
        const triggerIndex = triggerMap[i][2];
        const type = triggerMap[i][3];
        const delta = triggerMap[i][4];

        const simulationWorkerPlus = new Worker(new URL('./combatSimulator.js', import.meta.url));
        simulatorWorkerPool.push(simulationWorkerPlus);
        simulationWorkerPlus.postMessage({
            type: "start_simulation",
            players: tweakPlayersTrigger(players, playerIndex, itemIndex, triggerIndex, delta, type),
            zone: zoneHrid,
            simulationTimeLimit: simulationTimeHours * ONE_HOUR,
            simId: `${i}+`,
        });

        const simulationWorkerMinus = new Worker(new URL('./combatSimulator.js', import.meta.url));
        simulatorWorkerPool.push(simulationWorkerMinus);
        simulationWorkerMinus.postMessage({
            type: "start_simulation",
            players: tweakPlayersTrigger(players, playerIndex, itemIndex, triggerIndex, -delta, type),
            zone: zoneHrid,
            simulationTimeLimit: simulationTimeHours * ONE_HOUR,
            simId: `${i}-`,
        });
    }

    // Wait for all simulations to finish
    const simulationResults = await Promise.all(simulatorWorkerPool.map(worker => {
        return new Promise((resolve, reject) => {
            worker.onmessage = function (event) {
                if (event.data.type === "simulation_result") {
                    resolve(event.data.simResult);
                } else if (event.data.type === "simulation_error") {
                    reject(event.data.error);
                }
            };
        });
    }));

    // Terminate all workers
    for (let i = 0; i < simulatorWorkerPool.length; i++) {
        simulatorWorkerPool[i].terminate();
    }

    // Process simulation results by simId
    let computeResults = {};
    for (let i = 0; i < simulationResults.length; i++) {
        if (optimizeTarget == "EPH") {
            computeResults[simulationResults[i].simId] = OPTIMIZABLE_TARGETS['encounters'](simulationResults[i]) / simulationTimeHours;
        } else {
            computeResults[simulationResults[i].simId] = OPTIMIZABLE_TARGETS[optimizeTarget](simulationResults[i]);
        }
    }

    const maxValue = Math.max(...Object.values(computeResults));

    // Calculate gradients
    for (let i = 0; i < args.length; i++) {
        const valuePlus = computeResults[`${i}+`];
        const valueMinus = computeResults[`${i}-`];
        const delta = triggerMap[i][4];
        gradients.push((valuePlus - valueMinus) / (2 * delta));
    }
    return [gradients, maxValue];
}

function tweakPlayersTrigger(players, playerIndex, itemIndex, triggerIndex, delta, type = "ability") {
    players = structuredClone(players);
    if (type == "abilities") {
        players[playerIndex].abilities[itemIndex].triggers[triggerIndex].value += delta
    } else if (type == "food") {
        players[playerIndex].food[itemIndex].triggers[triggerIndex].value += delta
    } else if (type == "drinks") {
        players[playerIndex].drinks[itemIndex].triggers[triggerIndex].value += delta
    }
    return players;
}

function getPlayersOptimizableTriggersMap(players) {
    // Get all triggers that can be optimized
    // and return an array of [[playerIndex, itemIndex, triggerIndex, type, delta]]
    const types = ["abilities", "food", "drinks"];
    let triggersMap = [];
    let args = [];

    for (let i = 0; i < players.length; i++) {
        const player = players[i];
        for (const type of types) {
            for (let j = 0; j < player[type].length; j++) {
                const item = player[type][j];
                if (!item || !item.triggers) continue;
                for (let k = 0; k < item.triggers.length; k++) {
                    const trigger = item.triggers[k];
                    if (OPTIMIZABLE_TRIGGERS[trigger.conditionHrid]) {
                        const min = OPTIMIZABLE_TRIGGERS[trigger.conditionHrid][0];
                        const max = OPTIMIZABLE_TRIGGERS[trigger.conditionHrid][1];
                        const delta = OPTIMIZABLE_TRIGGERS[trigger.conditionHrid][2];
                        triggersMap.push([i, j, k, type, delta, min, max]);
                        args.push(trigger.value);
                    }
                }
            }
        }
    }
    return [triggersMap, args];
}

function getDetailFromTriggerMap(players, triggerMap, args){
    let res = [];
    for (let i = 0; i < triggerMap.length; i++) {
        const playerIndex = triggerMap[i][0];
        const itemIndex = triggerMap[i][1];
        const triggerIndex = triggerMap[i][2];
        const type = triggerMap[i][3];
        const delta = triggerMap[i][4];

        res.push({
            player: players[playerIndex],
            item: players[playerIndex][type][itemIndex],
            trigger: players[playerIndex][type][itemIndex].triggers[triggerIndex],
            value: args[i],
            delta: delta,
        });
    }
    return res;
}


onmessage = async function (event) {
    switch (event.data.type) {
        case "start_optimization":
            let playersData = event.data.players;
            let zoneHrid = event.data.zoneHrid;
            let optimizeTarget = event.data.optimizeTarget;
            let triggerResults = await triggerOptimizer(playersData, zoneHrid, optimizeTarget);
            break;
    }
}
