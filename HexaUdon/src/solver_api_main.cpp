#include <iostream>

#include <nlohmann/json.hpp>

#include "solver/SolverApi.hpp"

using json = nlohmann::json;

int main() {
    try {
        json request;
        std::cin >> request;
        const std::string operation = request.value("operation", "solve");
        const json response = operation == "agent-types"
            ? SolverApi::agentTypes(request)
            : SolverApi::solve(request);
        std::cout << response.dump() << std::endl;
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
}