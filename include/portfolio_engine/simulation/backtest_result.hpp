#pragma once

#include "portfolio_engine/core/types.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <string>
#include <vector>

namespace portfolio_engine {

/**
 * @brief Performance and trade metrics for a completed strategy backtest.
 */
struct BacktestResult {
    std::string strategy_name;
    std::vector<double> portfolio_values;
    RebalanceStats stats;

    double initial_value() const noexcept {
        return portfolio_values.empty() ? 0.0 : portfolio_values.front();
    }

    double final_value() const noexcept {
        return portfolio_values.empty() ? 0.0 : portfolio_values.back();
    }

    double total_return_pct() const noexcept {
        double init = initial_value();
        return (init > 0.0) ? ((final_value() - init) / init * 100.0) : 0.0;
    }

    double max_value() const noexcept {
        if (portfolio_values.empty()) return 0.0;
        return *std::max_element(portfolio_values.begin(), portfolio_values.end());
    }

    double min_value() const noexcept {
        if (portfolio_values.empty()) return 0.0;
        return *std::min_element(portfolio_values.begin(), portfolio_values.end());
    }

    double max_drawdown_pct() const noexcept {
        if (portfolio_values.empty()) return 0.0;
        double peak = portfolio_values[0];
        double max_dd = 0.0;
        for (double v : portfolio_values) {
            if (v > peak) peak = v;
            double dd = (peak > 0.0) ? ((peak - v) / peak) : 0.0;
            if (dd > max_dd) max_dd = dd;
        }
        return max_dd * 100.0;
    }

    double daily_volatility() const noexcept {
        if (portfolio_values.size() <= 2) return 0.0;
        size_t n = portfolio_values.size() - 1;
        std::vector<double> ret(n);
        double sum = 0.0;
        for (size_t t = 0; t < n; ++t) {
            double prev = portfolio_values[t];
            double curr = portfolio_values[t + 1];
            ret[t] = (prev > 0.0) ? ((curr - prev) / prev) : 0.0;
            sum += ret[t];
        }
        double mean = sum / static_cast<double>(n);
        double sq_sum = 0.0;
        for (double r : ret) {
            sq_sum += (r - mean) * (r - mean);
        }
        return std::sqrt(sq_sum / static_cast<double>(n - 1));
    }

    double sharpe_ratio(double annual_factor = 252.0) const noexcept {
        double vol = daily_volatility();
        if (vol <= 1e-12) return 0.0;
        size_t n = portfolio_values.size() - 1;
        double sum = 0.0;
        for (size_t t = 0; t < n; ++t) {
            double prev = portfolio_values[t];
            double curr = portfolio_values[t + 1];
            sum += (prev > 0.0) ? ((curr - prev) / prev) : 0.0;
        }
        double mean = sum / static_cast<double>(n);
        return (mean / vol) * std::sqrt(annual_factor);
    }

    static void export_csv(const std::string& filename,
                           const std::vector<BacktestResult>& results) {
        if (results.empty()) return;
        std::ofstream out(filename);
        if (!out.is_open()) return;

        out << std::fixed << std::setprecision(2);
        out << "Step";
        for (const auto& res : results) {
            out << "," << res.strategy_name;
        }
        out << "\n";

        size_t timesteps = results[0].portfolio_values.size();
        for (size_t t = 0; t < timesteps; ++t) {
            out << t;
            for (const auto& res : results) {
                out << "," << (t < res.portfolio_values.size() ? res.portfolio_values[t] : 0.0);
            }
            out << "\n";
        }
    }
};

} // namespace portfolio_engine
