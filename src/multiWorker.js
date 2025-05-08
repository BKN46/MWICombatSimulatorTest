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
                        const simulationWorker = new Worker(new URL('worker.js', import.meta.url));
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