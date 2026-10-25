#include "portfolio_engine/core/portfolio.hpp"
#include "portfolio_engine/core/types.hpp"
#include "portfolio_engine/optimization/gurobi_solver.hpp"
#include "portfolio_engine/simulation/simulation_engine.hpp"
#include "portfolio_engine/strategies/buy_and_hold.hpp"
#include "portfolio_engine/strategies/fixed_weight_strategy.hpp"
#include "portfolio_engine/strategies/investor_view_strategy.hpp"
#include "portfolio_engine/strategies/markowitz_strategy.hpp"
#include "portfolio_engine/strategies/simplex_lp_strategy.hpp"
#include "portfolio_engine/strategies/threshold_strategy.hpp"
#include <iostream>
#include <memory>

using namespace portfolio_engine;

int main() {
    std::cout << "===================================================================\n";
    std::cout << "           PORTFOLIO ENGINE - QUANTITATIVE BACKTEST SUITE          \n";
    std::cout << "===================================================================\n";

    // 1. Setup 3-Asset Academic Benchmark Preset
    auto assets = BenchmarkPreset::default_assets();
    constexpr double starting_cash = BenchmarkPreset::DEFAULT_STARTING_CASH;
    constexpr size_t timesteps = BenchmarkPreset::DEFAULT_TIMESTEPS;
    constexpr unsigned int seed = BenchmarkPreset::DEFAULT_SEED;

    std::cout << "Target Assets:\n";
    for (const auto& a : assets) {
        std::cout << "  - " << a.symbol << ": " << a.initial_shares << " shares @ $"
                  << a.initial_price << " (mu=" << a.mu << ", sigma=" << a.sigma << ")\n";
    }
    std::cout << "Initial Cash: $" << starting_cash << "\n";
    std::cout << "Simulation Horizon: " << timesteps << " steps\n";
    std::cout << "Random Seed: " << seed << "\n";
    std::cout << "Gurobi Optimization: " << (GurobiSolver::is_available() ? "ENABLED (QP/MILP/MIQP active)" : "DISABLED (Native Simplex active)") << "\n\n";

    // 2. Initialize Engine
    SimulationEngine engine(assets, starting_cash, seed);

    // 3. Register Native Core Strategies
    engine.add_strategy(std::make_shared<BuyAndHoldStrategy>());
    engine.add_strategy(std::make_shared<ThresholdTradingStrategy>(10, 0.05, 0.10));
    engine.add_strategy(std::make_shared<FixedWeightStrategy>());
    engine.add_strategy(std::make_shared<InvestorViewStrategy>());
    engine.add_strategy(std::make_shared<SimplexLPStrategy>());

    // 4. Register Gurobi Strategies if enabled
    if (GurobiSolver::is_available()) {
        engine.add_strategy(std::make_shared<MarkowitzQPStrategy>());
        engine.add_strategy(std::make_shared<CardinalityILPStrategy>());
        engine.add_strategy(std::make_shared<SemiContinuousMILPStrategy>());
        engine.add_strategy(std::make_shared<MarkowitzMIQPStrategy>());
    }

    // 5. Execute Simulation
    std::cout << "Running backtest over " << timesteps << " steps...\n";
    auto results = engine.run(timesteps);

    // 6. Display Analytics Table
    SimulationEngine::print_comparison_table(results);

    // 7. Export Trajectory Data
    const std::string csv_file = "portfolio_comparison.csv";
    BacktestResult::export_csv(csv_file, results);
    std::cout << "Portfolio trajectory data exported successfully to: " << csv_file << "\n";
    std::cout << "Simulation completed successfully.\n";

    return 0;
}
