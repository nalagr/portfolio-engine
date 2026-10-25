#pragma once

#include "portfolio_engine/core/types.hpp"
#include "portfolio_engine/simulation/backtest_result.hpp"
#include "portfolio_engine/simulation/gbm_simulator.hpp"
#include "portfolio_engine/strategies/strategy.hpp"
#include <memory>
#include <vector>

namespace portfolio_engine {

/**
 * @brief Multi-strategy backtesting orchestrator. Runs identical simulated price
 *        paths across all registered strategies and computes benchmark analytics.
 */
class SimulationEngine {
public:
    SimulationEngine(std::vector<AssetParams> assets,
                     double starting_cash = BenchmarkPreset::DEFAULT_STARTING_CASH,
                     unsigned int seed = BenchmarkPreset::DEFAULT_SEED);

    /// Register a new strategy to evaluate
    void add_strategy(std::shared_ptr<Strategy> strategy);

    /// Clear all registered strategies
    void clear_strategies();

    /// Execute the backtest over T steps
    std::vector<BacktestResult> run(size_t timesteps = BenchmarkPreset::DEFAULT_TIMESTEPS);

    /// Print comprehensive comparative performance metrics to stdout
    static void print_comparison_table(const std::vector<BacktestResult>& results);

    const std::vector<AssetParams>& assets() const noexcept { return assets_; }
    double starting_cash() const noexcept { return starting_cash_; }

private:
    std::vector<AssetParams> assets_;
    double starting_cash_;
    unsigned int seed_;
    std::vector<std::shared_ptr<Strategy>> strategies_;
};

} // namespace portfolio_engine
