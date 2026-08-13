#include "solver/Solver.hpp"
#include <iostream>

/**
 * @brief Quyet dinh loai xe (0: Xe tuan tra, 1: Xe tiep te)
 * 
 * TODO [USER]: Ban co the thay doi chien thuat phan bo loai xe o day.
 * Mua xe Patrol hay Supply tuy thuoc vao luong gian hang va chien thuat cua doi.
 */
std::vector<int> AgentStrategy::decideAgentTypes(const GameConfig& config) {
    size_t numAgents = config.initialAgentPositions.size();
    
    // TODO [USER]: Viet logic chon xe o day nếu muon.
    // Vi du: set xe dau la Supply (1), cac xe sau la Patrol (0)
    std::vector<int> agentTypes(numAgents, 0);
    return agentTypes;
}

std::vector<int> Solver::decideAgentTypes(const GameConfig& config) {
    return AgentStrategy::decideAgentTypes(config);
}

/**
 * =========================================================================
 * @brief HAM CHINH VIET LOGIC DI CHUYEN CHO CAC AGENT (SOLVER CORE)
 * =========================================================================
 * 
 * @param config  Chua thong tin co dinh (ban do, spots, daySteps...)
 * @param state   Chua thong tin thoi gian thuc (ngay hien tai, vi tri/fuel cac xe, traffic...)
 * @param map     Doi tuong Map ho tro: posToCoordinate, nextPosition, canMove...
 * @return std::vector<std::vector<int>> Chuoi hanh dong cua tung xe
 * 
 * LOGIC HOAT DONG HUONG DAN:
 * 1. Lay so buoc (daySteps) cua ngay hien tai: config.daySteps[state.day]
 * 2. Voi moi Agent (state.agents[i]):
 *    - Lấy vi tri hien tai: state.agents[i].pos -> chuyen sang toa do 2D `map.posToCoordinate(...)`
 *    - Lay luong nhien lieu con lai: state.agents[i].fuel
 *    - Tinh toan dich den va duong đi (Co the dung `PathFinder::findPath(...)`)
 *    - Sinh mảng hanh dong (0..5: di chuyen, -K: cho K buoc)
 * 3. Đảm bảo tong so buoc cua moi Agent == daySteps
 */
std::vector<std::vector<int>> Solver::solve(
    const GameConfig& config,
    const GameState& state,
    const Map& map
) {
    int daySteps = 0;
    if (state.day >= 0 && state.day < static_cast<int>(config.daySteps.size())) {
        daySteps = config.daySteps[state.day];
    }

    std::vector<std::vector<int>> actions(state.agents.size());

    // =====================================================================
    // TODO [USER]: VIET LOGIC DI CHUYEN CHO NGUOI DUNG TAI DAY
    // =====================================================================
    // Hien tai ham dang de trong va dung logic di chuyen 1 buoc an toan + cho
    // de ban de dang ghi de va phat trien thuat toan rieng.
    // ---------------------------------------------------------------------

    for (size_t i = 0; i < state.agents.size(); ++i) {
        Position currPos = map.posToCoordinate(state.agents[i].pos);
        
        // TODO [USER]: Thay the doan code tim huong ngau nhien/hoplê duoi đây
        // bang thuat toan tim duong thuc su (A*, Dijkstra, BFS toi gian hang...)
        int validDir = -1;
        for (int dir = 0; dir < 6; ++dir) {
            Position nxt = map.nextPosition(currPos, dir);
            if (map.canMove(nxt)) {
                validDir = dir;
                break;
            }
        }

        if (validDir != -1 && daySteps > 1) {
            // Hanh dong vi du: Đi 1 buoc va cho (daySteps - 1) buoc
            actions[i].push_back(validDir);
            actions[i].push_back(-(daySteps - 1));
        } else {
            // Dung yen toan bo ngay neu khong co duong di
            if (daySteps > 0) {
                actions[i].push_back(-daySteps);
            }
        }
    }

    // =====================================================================
    return actions;
}

/**
 * @brief Ke hoach an toan (Dung yen) khi validate bi loi
 */
std::vector<std::vector<int>> Solver::createFallbackActions(
    const GameConfig& config,
    const GameState& state
) {
    int daySteps = 0;
    if (state.day >= 0 && state.day < static_cast<int>(config.daySteps.size())) {
        daySteps = config.daySteps[state.day];
    }

    std::vector<std::vector<int>> fallback(state.agents.size());
    for (size_t i = 0; i < state.agents.size(); ++i) {
        if (daySteps > 0) {
            fallback[i].push_back(-daySteps);
        }
    }
    return fallback;
}
