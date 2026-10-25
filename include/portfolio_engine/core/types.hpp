#pragma once

#include <string>
#include <vector>

namespace portfolio_engine {

/**
 * @brief Parameters for stochastic price simulation of an asset.
 */
struct AssetParams {
    std::string symbol;
    double mu = 0.0;           ///< Expected daily return (drift)
    double sigma = 0.0;        ///< Return volatility (std dev)
    int initial_shares = 0;    ///< Initial share count
    double initial_price = 0.0;///< Initial price per share
};

/**
 * @brief Execution and turnover statistics for portfolio rebalancing.
 */
struct RebalanceStats {
    int buys = 0;              ///< Count of buy operations
    int sells = 0;             ///< Count of sell operations
    double dollar_volume = 0.0;///< Cumulative dollar amount traded
    int rebalance_events = 0;  ///< Number of rebalance check triggers
};

/**
 * @brief Default benchmark settings replicating the 3-asset academic setup.
 */
struct BenchmarkPreset {
    static constexpr double DEFAULT_STARTING_CASH = 100000.0;
    static constexpr size_t DEFAULT_TIMESTEPS = 1000;
    static constexpr size_t DEFAULT_REBALANCE_PERIOD = 100;
    static constexpr unsigned int DEFAULT_SEED = 42;

    static std::vector<AssetParams> default_assets() {
        return {
            {"NVDA", 0.0008, 0.030, 110, 78.54},
            {"AAPL", 0.0004, 0.015, 200, 189.72},
            {"GOOG", 0.0005, 0.020, 100, 173.88}
        };
    }
};

} // namespace portfolio_engine
