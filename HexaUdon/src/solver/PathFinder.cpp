#include "solver/PathFinder.hpp"
#include <queue>
#include <map>
#include <algorithm>

std::vector<int> PathFinder::findPath(
    Position start,
    Position goal,
    const Map& map
) {
    if (start == goal) return {};

    std::queue<Position> q;
    std::map<std::pair<int, int>, std::pair<Position, int>> parent; // (x,y) -> (prev_pos, dir)
    std::map<std::pair<int, int>, bool> visited;

    q.push(start);
    visited[{start.x, start.y}] = true;

    bool found = false;

    while (!q.empty()) {
        Position curr = q.front();
        q.pop();

        if (curr == goal) {
            found = true;
            break;
        }

        for (int dir = 0; dir < 6; ++dir) {
            Position nxt = map.nextPosition(curr, dir);
            if (map.canMove(nxt) && !visited[{nxt.x, nxt.y}]) {
                visited[{nxt.x, nxt.y}] = true;
                parent[{nxt.x, nxt.y}] = {curr, dir};
                q.push(nxt);
            }
        }
    }

    if (!found) return {};

    std::vector<int> pathDirs;
    Position curr = goal;

    while (!(curr == start)) {
        auto pInfo = parent[{curr.x, curr.y}];
        pathDirs.push_back(pInfo.second);
        curr = pInfo.first;
    }

    std::reverse(pathDirs.begin(), pathDirs.end());
    return pathDirs;
}
