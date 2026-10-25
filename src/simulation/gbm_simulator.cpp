#include "portfolio_engine/simulation/gbm_simulator.hpp"
#include <algorithm>
#include <stdexcept>

namespace portfolio_engine {

GeometricBrownianMotion::GeometricBrownianMotion(std::vector<AssetParams> assets,
                                                 unsigned int seed)
    : assets_(std::move(assets)),
      initial_seed_(seed),
      rng_(seed) {
    current_prices_.reserve(assets_.size());
    for (const auto& a : assets_) {
        current_prices_.push_back(a.initial_price);
    }
}

std::vector<double> GeometricBrownianMotion::step() {
    for (size_t i = 0; i < assets_.size(); ++i) {
        double rt = assets_[i].mu + assets_[i].sigma * std_norm_(rng_);
        double next_p = std::max(0.0, current_prices_[i] * (1.0 + rt));
        current_prices_[i] = next_p;
    }
    return current_prices_;
}

std::vector<std::vector<double>> GeometricBrownianMotion::simulate_trajectory(size_t T) {
    reset();
    std::vector<std::vector<double>> path(T + 1, std::vector<double>(assets_.size()));
    path[0] = current_prices_;

    for (size_t t = 1; t <= T; ++t) {
        path[t] = step();
    }
    return path;
}

void GeometricBrownianMotion::reset() {
    rng_.seed(initial_seed_);
    for (size_t i = 0; i < assets_.size(); ++i) {
        current_prices_[i] = assets_[i].initial_price;
    }
}

std::vector<std::vector<double>> GeometricBrownianMotion::compute_covariance(
    const std::vector<std::vector<double>>& price_history,
    size_t current_step,
    size_t window) {

    if (current_step < window) {
        throw std::invalid_argument("Insufficient price history for covariance calculation");
    }
    size_t n = price_history[0].size();
    std::vector<std::vector<double>> returns(window, std::vector<double>(n, 0.0));
    std::vector<double> mean(n, 0.0);

    for (size_t i = 0; i < window; ++i) {
        size_t prev_t = current_step - window + i;
        size_t curr_t = prev_t + 1;
        for (size_t j = 0; j < n; ++j) {
            double p_prev = price_history[prev_t][j];
            double p_curr = price_history[curr_t][j];
            double ret = (p_prev > 0.0) ? ((p_curr - p_prev) / p_prev) : 0.0;
            returns[i][j] = ret;
            mean[j] += ret / static_cast<double>(window);
        }
    }

    std::vector<std::vector<double>> Q(n, std::vector<double>(n, 0.0));
    double denom = static_cast<double>(window - 1);
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < window; ++k) {
                sum += (returns[k][i] - mean[i]) * (returns[k][j] - mean[j]);
            }
            Q[i][j] = sum / denom;
        }
    }
    return Q;
}

std::vector<double> GeometricBrownianMotion::compute_lookback_returns(
    const std::vector<std::vector<double>>& price_history,
    size_t current_step,
    size_t lookback) {

    if (current_step < lookback) {
        throw std::invalid_argument("Insufficient price history for lookback returns");
    }
    size_t n = price_history[0].size();
    std::vector<double> mu(n, 0.0);
    size_t base_step = current_step - lookback;

    for (size_t j = 0; j < n; ++j) {
        double p_base = price_history[base_step][j];
        double p_curr = price_history[current_step][j];
        mu[j] = (p_base > 0.0) ? ((p_curr - p_base) / p_base) : 0.0;
    }
    return mu;
}

} // namespace portfolio_engine
