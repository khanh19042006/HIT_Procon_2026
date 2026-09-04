#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

#include "io/JsonReader.hpp"
#include "io/JsonWriter.hpp"
#include "map/Map.hpp"
#include "solver/Solver.hpp"
#include "solver/ActionValidator.hpp"
#include "api/GameApiClient.hpp"

// =============================================================================
// MODE 1: API mode — Connect to Procon server via HTTPS REST API
// =============================================================================
int runApiMode(const std::string& serverUrl, const std::string& token, const std::string& matchId) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif

    std::cout << "========================================================================\n";
    std::cout << "  HEXAUDON BOT v2.0 — CHE DO THI DAU (API MODE)\n";
    std::cout << "========================================================================\n";
    std::cout << "  Server : " << serverUrl << "\n";
    std::cout << "  Match  : " << matchId << "\n";
    std::cout << "========================================================================\n\n";

    GameApiClient api(serverUrl, token);

    // --- Step 1: Get match config ---
    std::cout << "[1/4] Dang lay cau hinh tran dau (GET /config)...\n";
    GameConfig config = api.getMatchConfig(matchId);
    if (config.daySteps.empty()) {
        std::cerr << "[LOI] Khong lay duoc config: " << api.getLastError() << "\n";
        return 1;
    }
    std::cout << "  -> Ban do: " << config.map.width << "x" << config.map.height << "\n";
    std::cout << "  -> So xe: " << config.initialAgentPositions.size() << "\n";
    std::cout << "  -> So spot: " << config.spots.size() << "\n";
    std::cout << "  -> So ngay: " << config.daySteps.size() << "\n";
    std::cout << "  -> Fuel limit: " << config.fuelLimit << "\n\n";

    Map map(config.map.height, config.map.width, config.map.cells);
    Solver solver;

    // --- Step 2: Submit agent types ---
    std::cout << "[2/4] Dang gui lua chon loai xe (POST /agents)...\n";
    auto agentTypes = solver.decideAgentTypes(config);
    std::cout << "  -> Loai xe: [";
    for (size_t i = 0; i < agentTypes.size(); ++i) {
        std::cout << agentTypes[i];
        if (i + 1 < agentTypes.size()) std::cout << ", ";
    }
    std::cout << "]\n";

    if (!api.submitAgentTypes(matchId, agentTypes)) {
        std::cerr << "[LOI] Khong gui duoc agent types: " << api.getLastError() << "\n";
        // Not fatal — might already be submitted
        std::cout << "  -> Canh bao: Co the da gui truoc do, tiep tuc...\n";
    } else {
        std::cout << "  -> Da gui thanh cong!\n";
    }
    std::cout << "\n";

    // --- Step 3: Main loop — poll status and submit actions for each day ---
    std::cout << "[3/4] Bat dau vong lap thi dau...\n\n";

    int lastDay = -1;
    int totalDays = static_cast<int>(config.daySteps.size());

    while (true) {
        // Poll match status
        GameState state = api.getMatchStatus(matchId);

        if (state.day < 0) {
            // Not started yet or error — wait and retry
            std::cout << "  [Cho] Tran dau chua bat dau hoac loi... thu lai sau 2 giay\n";
            std::cout << "        (" << api.getLastError() << ")\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        // Check if match ended
        if (state.day >= totalDays) {
            std::cout << "\n[4/4] Tran dau da ket thuc! (day=" << state.day << " >= " << totalDays << ")\n";
            break;
        }

        // Skip if we already submitted for this day
        if (state.day == lastDay) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }

        // New day!
        std::cout << "------------------------------------------------------------------------\n";
        std::cout << "  NGAY " << state.day << "/" << totalDays - 1
                  << " (Steps: " << config.daySteps[state.day] << ")\n";
        std::cout << "------------------------------------------------------------------------\n";

        // Print agent states
        for (size_t i = 0; i < state.agents.size(); ++i) {
            Position pos = map.posToCoordinate(state.agents[i].pos);
            std::cout << "  Xe #" << i
                      << " | pos=" << state.agents[i].pos
                      << " (" << pos.x << "," << pos.y << ")"
                      << " | fuel=" << state.agents[i].fuel
                      << " | kind=" << (state.agents[i].kind == 0 ? "Patrol" : "Supply")
                      << "\n";
        }

        // Solve
        auto actions = solver.solve(config, state, map);

        // Validate
        if (!ActionValidator::validate(config, state, actions, map)) {
            std::cerr << "  [CANH BAO] Action KHONG HOP LE! Dung fallback.\n";
            actions = solver.createFallbackActions(config, state);
        } else {
            std::cout << "  -> Ket qua: HOP LE\n";
        }

        // Print actions summary
        for (size_t i = 0; i < actions.size(); ++i) {
            int moveCount = 0;
            for (int a : actions[i]) {
                if (a >= 0 && a <= 5) moveCount++;
            }
            std::cout << "  Xe #" << i << ": " << moveCount << " moves, "
                      << actions[i].size() << " total actions\n";
        }

        // Submit
        if (api.submitActions(matchId, actions)) {
            std::cout << "  -> DA GUI THANH CONG!\n\n";
            lastDay = state.day;
        } else {
            std::cerr << "  [LOI] Gui that bai: " << api.getLastError() << "\n";
            std::cerr << "  -> Thu lai sau 1 giay...\n\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue; // Retry this day
        }
    }

    std::cout << "========================================================================\n";
    std::cout << "  HOAN THANH THI DAU!\n";
    std::cout << "========================================================================\n";

    return 0;
}

// =============================================================================
// MODE 2: Stdin mode — Read from stdin, write to stdout (for local testing)
// =============================================================================
int runStdinMode() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    GameConfig config = JsonReader::readGameConfig();
    Map map(config.map.height, config.map.width, config.map.cells);
    Solver solver;

    auto agentTypes = solver.decideAgentTypes(config);
    JsonWriter::writeAgentTypes(agentTypes);

    for (size_t day = 0; day < config.daySteps.size(); ++day) {
        GameState state = JsonReader::readGameState();
        auto actions = solver.solve(config, state, map);

        if (!ActionValidator::validate(config, state, actions, map)) {
            std::cerr << "[WARNING] Day " << day << " invalid! Using fallback.\n";
            actions = solver.createFallbackActions(config, state);
        }

        JsonWriter::writeActions(actions);
    }

    return 0;
}

// =============================================================================
// MAIN — Parse arguments and run appropriate mode
// =============================================================================
void printUsage(const char* prog) {
    std::cout << "HexaUdon Bot v2.0 — Procon 2026\n\n";
    std::cout << "Cach dung:\n\n";
    std::cout << "  CHE DO THI DAU (ket noi server):\n";
    std::cout << "    " << prog << " --server URL --token TOKEN --match MATCH_ID\n\n";
    std::cout << "  Vi du:\n";
    std::cout << "    " << prog << " --server https://procon26.haui.ac.vn --token abc123 --match 6789\n\n";
    std::cout << "  CHE DO LOCAL (stdin/stdout):\n";
    std::cout << "    " << prog << " --stdin\n";
    std::cout << "    type demo_input.json | " << prog << " --stdin\n\n";
}

int main(int argc, char* argv[]) {
    std::string serverUrl;
    std::string token;
    std::string matchId;
    bool stdinMode = false;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--server" && i + 1 < argc) {
            serverUrl = argv[++i];
        } else if (arg == "--token" && i + 1 < argc) {
            token = argv[++i];
        } else if (arg == "--match" && i + 1 < argc) {
            matchId = argv[++i];
        } else if (arg == "--stdin") {
            stdinMode = true;
        } else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        }
    }

    // If no arguments → show help
    if (argc <= 1) {
        printUsage(argv[0]);
        return 0;
    }

    if (stdinMode) {
        return runStdinMode();
    }

    if (serverUrl.empty() || token.empty() || matchId.empty()) {
        std::cerr << "[LOI] Thieu tham so! Can: --server, --token, --match\n\n";
        printUsage(argv[0]);
        return 1;
    }

    return runApiMode(serverUrl, token, matchId);
}