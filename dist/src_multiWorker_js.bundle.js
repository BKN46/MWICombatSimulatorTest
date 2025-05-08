/******/ (() => { // webpackBootstrap
/******/ 	var __webpack_modules__ = ({});
/************************************************************************/
/******/ 	// The require scope
/******/ 	var __webpack_require__ = {};
/******/ 	
/******/ 	// expose the modules object (__webpack_modules__)
/******/ 	__webpack_require__.m = __webpack_modules__;
/******/ 	
/************************************************************************/
/******/ 	/* webpack/runtime/get javascript chunk filename */
/******/ 	(() => {
/******/ 		// This function allow to reference async chunks
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
/******/ 			"src_multiWorker_js": 1
/******/ 		};
/******/ 		
/******/ 		// no chunk install function needed
/******/ 		// no chunk loading
/******/ 		
/******/ 		// no HMR
/******/ 		
/******/ 		// no HMR manifest
/******/ 	})();
/******/ 	
/************************************************************************/
/*!****************************!*\
  !*** ./src/multiWorker.js ***!
  \****************************/
onmessage = async function (event) {
    switch (event.data.type) {
        case "start_simulation_all_zones":
            // get now time
            const now = new Date();
            const zoneHrids = event.data.zones;
            let zoneProgress = {};

            // 设置 worker 池最大数量
            const threadLimit = event.data.threadLimit ? event.data.threadLimit : 4;
            console.log(`Multi worker run in ${threadLimit} threads`);
            
            try {
                const outer_worker = this;
                const allSimulationResults = [];
                
                // Deal with cases by batches
                for (let startIdx = 0; startIdx < zoneHrids.length; startIdx += threadLimit) {
                    const simulatorWorkerPool = [];
                    const currentBatchSize = Math.min(threadLimit, zoneHrids.length - startIdx);
                    
                    // Create workers
                    for (let i = 0; i < currentBatchSize; i++) {
                        const zoneIndex = startIdx + i;
                        const workerId = Math.random().toString(36).substring(2, 15);
                        const simulationWorker = new Worker(new URL(/* worker import */ __webpack_require__.p + __webpack_require__.u(1), __webpack_require__.b));
                        simulatorWorkerPool.push({
                            id: workerId,
                            worker: simulationWorker,
                            zoneHrid: zoneHrids[zoneIndex]
                        });
                        
                        let workerMessage = {
                            type: "start_simulation",
                            players: event.data.players,
                            workerId: workerId,
                            zoneHrid: zoneHrids[zoneIndex],
                            simulationTimeLimit: event.data.simulationTimeLimit,
                        };
                        simulationWorker.postMessage(workerMessage);
                    }
                    
                    // Wait til all workers finish
                    const batchResults = await Promise.all(simulatorWorkerPool.map(workerInfo => {
                        return new Promise((resolve, reject) => {
                            workerInfo.worker.onmessage = function (event) {
                                if (event.data.type === "simulation_result") {
                                    resolve(event.data.simResult);
                                } else if (event.data.type === "simulation_progress") {
                                    zoneProgress[event.data.zone] = event.data.progress;
                                    let totalProgress = Object.values(zoneProgress).reduce((acc, progress) => acc + progress, 0) / zoneHrids.length;
                                    outer_worker.postMessage({ type: "simulation_progress", progress: totalProgress });
                                } else if (event.data.type === "simulation_error") {
                                    reject(event.data.error);
                                }
                            };
                        });
                    }));
                    
                    allSimulationResults.push(...batchResults);

                    for (let workerInfo of simulatorWorkerPool) {
                        workerInfo.worker.terminate();
                    }
                }
                
                this.postMessage({ type: "simulation_result_allZones", simResults: allSimulationResults });
            } catch (e) {
                console.log(e);
                this.postMessage({ type: "simulation_error", error: e });
            }
            break;
    }
};
/******/ })()
;
//# sourceMappingURL=src_multiWorker_js.bundle.js.map