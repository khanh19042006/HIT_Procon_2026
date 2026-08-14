#include "solver/SpotAssigner.hpp"
#include <algorithm>
#include <climits>

std::vector<int> SpotAssigner::assignSpotsToAgents(
    const std::vector<Agent>& agents,
    const std::vector<Spot>& spots,
    const Map& map
) {
    int numAgents = static_cast<int>(agents.size());
    int numSpots = static_cast<int>(spots.size());

    // Result: spotTarget[i] = spot index for agent i, -1 if none
    std::vector<int> spotTarget(numAgents, -1);

    if (numSpots == 0) return spotTarget;

    // Identify patrol car indices
    std::vector<int> patrolIndices;
    for (int i = 0; i < numAgents; ++i) {
        if (agents[i].kind == 0) {  // Patrol car
            patrolIndices.push_back(i);
        }
    }

    if (patrolIndices.empty()) return spotTarget;

    // Collect all unique brands
    std::set<int> allBrands;
    for (const auto& spot : spots) {
        allBrands.insert(spot.brand);
    }

    // Pre-compute distance from each patrol agent to each spot using Dijkstra
    // costMatrix[patrolIdx][spotIdx] = total steps to reach
    std::vector<std::vector<int>> costMatrix(patrolIndices.size(),
                                              std::vector<int>(numSpots, INT_MAX));

    for (size_t pi = 0; pi < patrolIndices.size(); ++pi) {
        int agentIdx = patrolIndices[pi];
        Position agentPos = map.posToCoordinate(agents[agentIdx].pos);

        for (int si = 0; si < numSpots; ++si) {
            Position spotPos = map.posToCoordinate(spots[si].pos);
            auto path = PathFinder::findPath(agentPos, spotPos, map, agents[agentIdx].fuel);
            if (path.found) {
                costMatrix[pi][si] = path.totalSteps;
            }
        }
    }

    // Track which spots and patrol agents are assigned
    std::vector<bool> spotAssigned(numSpots, false);
    std::vector<bool> patrolAssigned(patrolIndices.size(), false);

    // PHASE 1: Cover as many unique brands as possible
    // For each unrepresented brand, find the (patrol, spot) pair with lowest cost
    std::set<int> coveredBrands;

    for (int brand : allBrands) {
        // Find best (patrol, spot) pair for this brand
        int bestPatrol = -1;
        int bestSpot = -1;
        int bestCost = INT_MAX;

        for (size_t pi = 0; pi < patrolIndices.size(); ++pi) {
            if (patrolAssigned[pi]) continue;

            for (int si = 0; si < numSpots; ++si) {
                if (spotAssigned[si]) continue;
                if (spots[si].brand != brand) continue;
                if (spots[si].stocks <= 0) continue;

                if (costMatrix[pi][si] < bestCost) {
                    bestCost = costMatrix[pi][si];
                    bestPatrol = static_cast<int>(pi);
                    bestSpot = si;
                }
            }
        }

        if (bestPatrol >= 0 && bestSpot >= 0) {
            spotTarget[patrolIndices[bestPatrol]] = bestSpot;
            spotAssigned[bestSpot] = true;
            patrolAssigned[bestPatrol] = true;
            coveredBrands.insert(brand);
        }
    }

    // PHASE 2: Remaining patrol cars → assign closest unassigned spot with stock
    for (size_t pi = 0; pi < patrolIndices.size(); ++pi) {
        if (patrolAssigned[pi]) continue;

        int bestSpot = -1;
        int bestCost = INT_MAX;

        for (int si = 0; si < numSpots; ++si) {
            if (spotAssigned[si]) continue;
            if (spots[si].stocks <= 0) continue;

            if (costMatrix[pi][si] < bestCost) {
                bestCost = costMatrix[pi][si];
                bestSpot = si;
            }
        }

        if (bestSpot >= 0) {
            spotTarget[patrolIndices[pi]] = bestSpot;
            spotAssigned[bestSpot] = true;
            patrolAssigned[pi] = true;
        } else {
            // All spots assigned — go to nearest unassigned spot (even if assigned)
            bestCost = INT_MAX;
            for (int si = 0; si < numSpots; ++si) {
                if (spots[si].stocks <= 0) continue;
                if (costMatrix[pi][si] < bestCost) {
                    bestCost = costMatrix[pi][si];
                    bestSpot = si;
                }
            }
            if (bestSpot >= 0) {
                spotTarget[patrolIndices[pi]] = bestSpot;
                patrolAssigned[pi] = true;
            }
        }
    }

    return spotTarget;
}
