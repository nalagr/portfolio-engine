#pragma once

#include "portfolio_engine/core/types.hpp"
#include <random>
#include <vector>

namespace portfolio_engine {

/**
 * @brief Discrete-time Geometric Brownian Motion / stochastic walk price generator.
 *        Replicates exact homework stochastic dynamics:
 *        r_{i,t} = mu_i + sigma_i * epsilon_{i,t}, epsilon ~ N(0, 1)
 *        S_{i,t} = max(0.0, S_{i,t-1} * (1 + r_{i,t}))
 */
class GeometricBrownianMotion {
public:
    explicit GeometricBrownianMotion(std::vector<AssetParams> assets,
                                     unsigned int seed = BenchmarkPreset::DEFAULT_SEED);

    /// Advance one time step and return new asset prices
    std::vector<double> step();

    /// Simulate full trajectory of T steps; returns (T+1) x N price matrix
    std::vector<std::vector<double>> simulate_trajectory(size_t T);

    /// Reset simulator to initial seed and prices
    void reset();

    /// Compute rolling sample covariance matrix from past window prices (generalized for N assets)
    static std::vector<std::vector<double>> compute_covariance(
        const std::vector<std::vector<double>>& price_history,
        size_t current_step,
        size_t window = 100);

    /// Compute expected returns from lookback window
    static std::vector<double> compute_lookback_returns(
        const std::vector<std::vector<double>>& price_history,
        size_t current_step,
        size_t lookback = 100);

    const std::vector<double>& current_prices() const noexcept { return current_prices_; }
    const std::vector<AssetParams>& asset_params() const noexcept { return assets_; }

private:
    std::vector<AssetParams> assets_;
    unsigned int initial_seed_;
    std::mt19937 rng_;
    std::normal_distribution<double> std_norm_{0.0, 1.0};
    std::vector<double> current_prices_;
};

} // namespace portfolio_engine
