/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "./src/combatsimulator/optimizer.js":
/*!******************************************!*\
  !*** ./src/combatsimulator/optimizer.js ***!
  \******************************************/
/***/ ((__unused_webpack_module, __webpack_exports__, __webpack_require__) => {

__webpack_require__.r(__webpack_exports__);
/* harmony import */ var _player__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! ./player */ "./src/combatsimulator/player.js");


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

async function triggerOptimizer(players, zoneHrid, optimizeTarget, baseSimulationTime, learningRate){
    const [triggersMap, args] = getPlayersOptimizableTriggersMap(players, ["abilities"]);
    if (!triggersMap || triggersMap.length == 0) {
        console.log("No optimizable triggers found.");
        return null;
    }
    const result = await adamOptimizer(players, zoneHrid, triggersMap, args, optimizeTarget, baseSimulationTime, learningRate);
    return result;
}

async function adamOptimizer(players, zoneHrid, triggerMap, args, optimizeTarget="EPH", baseSimulationTime=24, learningRate = 1, beta1 = 0.9, beta2 = 0.999, epsilon = 1e-8, maxIterations = 5000) {
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

        const simulationWorkerPlus = new Worker(new URL(/* worker import */ __webpack_require__.p + __webpack_require__.u(2), __webpack_require__.b));
        simulatorWorkerPool.push(simulationWorkerPlus);
        simulationWorkerPlus.postMessage({
            type: "start_simulation",
            players: tweakPlayersTrigger(players, playerIndex, itemIndex, triggerIndex, delta, type),
            zone: zoneHrid,
            simulationTimeLimit: simulationTimeHours * ONE_HOUR,
            simId: `${i}+`,
        });

        const simulationWorkerMinus = new Worker(new URL(/* worker import */ __webpack_require__.p + __webpack_require__.u(1), __webpack_require__.b));
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

function getPlayersOptimizableTriggersMap(players, types) {
    // Get all triggers that can be optimized
    // and return an array of [[playerIndex, itemIndex, triggerIndex, type, delta]]
    types = types ? types : ["abilities", "food", "drinks"];
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
            let baseOptimizationTime = Math.floor(event.data.simulationTimeLimit / ONE_HOUR);
            let learningRate = event.data.learningRate;
            let triggerResults = await triggerOptimizer(playersData, zoneHrid, optimizeTarget, baseOptimizationTime, learningRate);
            break;
    }
}


/***/ })

/******/ 	});
/************************************************************************/
/******/ 	// The module cache
/******/ 	var __webpack_module_cache__ = {};
/******/ 	
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/ 		// Check if module is in cache
/******/ 		var cachedModule = __webpack_module_cache__[moduleId];
/******/ 		if (cachedModule !== undefined) {
/******/ 			return cachedModule.exports;
/******/ 		}
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = __webpack_module_cache__[moduleId] = {
/******/ 			// no module.id needed
/******/ 			// no module.loaded needed
/******/ 			exports: {}
/******/ 		};
/******/ 	
/******/ 		// Execute the module function
/******/ 		__webpack_modules__[moduleId](module, module.exports, __webpack_require__);
/******/ 	
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/ 	
/******/ 	// expose the modules object (__webpack_modules__)
/******/ 	__webpack_require__.m = __webpack_modules__;
/******/ 	
/******/ 	// the startup function
/******/ 	__webpack_require__.x = () => {
/******/ 		// Load entry module and return exports
/******/ 		// This entry module depends on other loaded chunks and execution need to be delayed
/******/ 		var __webpack_exports__ = __webpack_require__.O(undefined, ["src_combatsimulator_player_js"], () => (__webpack_require__("./src/combatsimulator/optimizer.js")))
/******/ 		__webpack_exports__ = __webpack_require__.O(__webpack_exports__);
/******/ 		return __webpack_exports__;
/******/ 	};
/******/ 	
/************************************************************************/
/******/ 	/* webpack/runtime/chunk loaded */
/******/ 	(() => {
/******/ 		var deferred = [];
/******/ 		__webpack_require__.O = (result, chunkIds, fn, priority) => {
/******/ 			if(chunkIds) {
/******/ 				priority = priority || 0;
/******/ 				for(var i = deferred.length; i > 0 && deferred[i - 1][2] > priority; i--) deferred[i] = deferred[i - 1];
/******/ 				deferred[i] = [chunkIds, fn, priority];
/******/ 				return;
/******/ 			}
/******/ 			var notFulfilled = Infinity;
/******/ 			for (var i = 0; i < deferred.length; i++) {
/******/ 				var [chunkIds, fn, priority] = deferred[i];
/******/ 				var fulfilled = true;
/******/ 				for (var j = 0; j < chunkIds.length; j++) {
/******/ 					if ((priority & 1 === 0 || notFulfilled >= priority) && Object.keys(__webpack_require__.O).every((key) => (__webpack_require__.O[key](chunkIds[j])))) {
/******/ 						chunkIds.splice(j--, 1);
/******/ 					} else {
/******/ 						fulfilled = false;
/******/ 						if(priority < notFulfilled) notFulfilled = priority;
/******/ 					}
/******/ 				}
/******/ 				if(fulfilled) {
/******/ 					deferred.splice(i--, 1)
/******/ 					var r = fn();
/******/ 					if (r !== undefined) result = r;
/******/ 				}
/******/ 			}
/******/ 			return result;
/******/ 		};
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/define property getters */
/******/ 	(() => {
/******/ 		// define getter functions for harmony exports
/******/ 		__webpack_require__.d = (exports, definition) => {
/******/ 			for(var key in definition) {
/******/ 				if(__webpack_require__.o(definition, key) && !__webpack_require__.o(exports, key)) {
/******/ 					Object.defineProperty(exports, key, { enumerable: true, get: definition[key] });
/******/ 				}
/******/ 			}
/******/ 		};
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/ensure chunk */
/******/ 	(() => {
/******/ 		__webpack_require__.f = {};
/******/ 		// This file contains only the entry chunk.
/******/ 		// The chunk loading function for additional chunks
/******/ 		__webpack_require__.e = (chunkId) => {
/******/ 			return Promise.all(Object.keys(__webpack_require__.f).reduce((promises, key) => {
/******/ 				__webpack_require__.f[key](chunkId, promises);
/******/ 				return promises;
/******/ 			}, []));
/******/ 		};
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/get javascript chunk filename */
/******/ 	(() => {
/******/ 		// This function allow to reference async chunks and sibling chunks for the entrypoint
/******/ 		__webpack_require__.u = (chunkId) => {
/******/ 			// return url for filenames based on template
/******/ 			return "" + chunkId + ".bundle.js";
/******/ 		};
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/global */
/******/ 	(() => {
/******/ 		__webpack_require__.g = (function() {
/******/ 			if (typeof globalThis === 'object') return globalThis;
/******/ 			try {
/******/ 				return this || new Function('return this')();
/******/ 			} catch (e) {
/******/ 				if (typeof window === 'object') return window;
/******/ 			}
/******/ 		})();
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/hasOwnProperty shorthand */
/******/ 	(() => {
/******/ 		__webpack_require__.o = (obj, prop) => (Object.prototype.hasOwnProperty.call(obj, prop))
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/make namespace object */
/******/ 	(() => {
/******/ 		// define __esModule on exports
/******/ 		__webpack_require__.r = (exports) => {
/******/ 			if(typeof Symbol !== 'undefined' && Symbol.toStringTag) {
/******/ 				Object.defineProperty(exports, Symbol.toStringTag, { value: 'Module' });
/******/ 			}
/******/ 			Object.defineProperty(exports, '__esModule', { value: true });
/******/ 		};
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/publicPath */
/******/ 	(() => {
/******/ 		var scriptUrl;
/******/ 		if (__webpack_require__.g.importScripts) scriptUrl = __webpack_require__.g.location + "";
/******/ 		var document = __webpack_require__.g.document;
/******/ 		if (!scriptUrl && document) {
/******/ 			if (document.currentScript && document.currentScript.tagName.toUpperCase() === 'SCRIPT')
/******/ 				scriptUrl = document.currentScript.src;
/******/ 			if (!scriptUrl) {
/******/ 				var scripts = document.getElementsByTagName("script");
/******/ 				if(scripts.length) {
/******/ 					var i = scripts.length - 1;
/******/ 					while (i > -1 && (!scriptUrl || !/^http(s?):/.test(scriptUrl))) scriptUrl = scripts[i--].src;
/******/ 				}
/******/ 			}
/******/ 		}
/******/ 		// When supporting browsers where an automatic publicPath is not supported you must specify an output.publicPath manually via configuration
/******/ 		// or pass an empty string ("") and set the __webpack_public_path__ variable from your code to use your own logic.
/******/ 		if (!scriptUrl) throw new Error("Automatic publicPath is not supported in this browser");
/******/ 		scriptUrl = scriptUrl.replace(/^blob:/, "").replace(/#.*$/, "").replace(/\?.*$/, "").replace(/\/[^\/]+$/, "/");
/******/ 		__webpack_require__.p = scriptUrl;
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/importScripts chunk loading */
/******/ 	(() => {
/******/ 		__webpack_require__.b = self.location + "";
/******/ 		
/******/ 		// object to store loaded chunks
/******/ 		// "1" means "already loaded"
/******/ 		var installedChunks = {
/******/ 			"src_combatsimulator_optimizer_js": 1
/******/ 		};
/******/ 		
/******/ 		// importScripts chunk loading
/******/ 		var installChunk = (data) => {
/******/ 			var [chunkIds, moreModules, runtime] = data;
/******/ 			for(var moduleId in moreModules) {
/******/ 				if(__webpack_require__.o(moreModules, moduleId)) {
/******/ 					__webpack_require__.m[moduleId] = moreModules[moduleId];
/******/ 				}
/******/ 			}
/******/ 			if(runtime) runtime(__webpack_require__);
/******/ 			while(chunkIds.length)
/******/ 				installedChunks[chunkIds.pop()] = 1;
/******/ 			parentChunkLoadingFunction(data);
/******/ 		};
/******/ 		__webpack_require__.f.i = (chunkId, promises) => {
/******/ 			// "1" is the signal for "already loaded"
/******/ 			if(!installedChunks[chunkId]) {
/******/ 				if(true) { // all chunks have JS
/******/ 					importScripts(__webpack_require__.p + __webpack_require__.u(chunkId));
/******/ 				}
/******/ 			}
/******/ 		};
/******/ 		
/******/ 		var chunkLoadingGlobal = self["webpackChunkmwicombatsimulator"] = self["webpackChunkmwicombatsimulator"] || [];
/******/ 		var parentChunkLoadingFunction = chunkLoadingGlobal.push.bind(chunkLoadingGlobal);
/******/ 		chunkLoadingGlobal.push = installChunk;
/******/ 		
/******/ 		// no HMR
/******/ 		
/******/ 		// no HMR manifest
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/startup chunk dependencies */
/******/ 	(() => {
/******/ 		var next = __webpack_require__.x;
/******/ 		__webpack_require__.x = () => {
/******/ 			return __webpack_require__.e("src_combatsimulator_player_js").then(next);
/******/ 		};
/******/ 	})();
/******/ 	
/************************************************************************/
/******/ 	
/******/ 	// run startup
/******/ 	var __webpack_exports__ = __webpack_require__.x();
/******/ 	
/******/ })()
;
//# sourceMappingURL=src_combatsimulator_optimizer_js.bundle.js.map