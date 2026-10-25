#include "portfolio_engine/simulation/simulation_engine.hpp"
#include <iomanip>
#include <iostream>

namespace portfolio_engine {

SimulationEngine::SimulationEngine(std::vector<AssetParams> assets,
                                   double starting_cash,
                                   unsigned int seed)
    : assets_(std::move(assets)),
      starting_cash_(starting_cash),
      seed_(seed) {}

void SimulationEngine::add_strategy(std::shared_ptr<Strategy> strategy) {
    if (strategy) {
        strategies_.push_back(std::move(strategy));
    }
}

void SimulationEngine::clear_strategies() {
    strategies_.clear();
}

std::vector<BacktestResult> SimulationEngine::run(size_t timesteps) {
    // 1. Generate underlying market price path
    GeometricBrownianMotion gbm(assets_, seed_);
    std::vector<std::vector<double>> price_path = gbm.simulate_trajectory(timesteps);

    std::vector<BacktestResult> results;
    results.reserve(strategies_.size());

    // 2. Evaluate each strategy on the identical price path
    for (const auto& strat : strategies_) {
        strat->reset();

        // Instantiate fresh portfolio matching starting configuration
        std::vector<Asset> initial_holdings;
        initial_holdings.reserve(assets_.size());
        for (const auto& a : assets_) {
            initial_holdings.emplace_back(a.symbol, a.initial_shares, a.initial_price);
        }
        Portfolio portfolio(std::move(initial_holdings), starting_cash_);

        BacktestResult res;
        res.strategy_name = strat->name();
        res.portfolio_values.reserve(timesteps + 1);
        res.portfolio_values.push_back(portfolio.compute_value());

        // Simulation loop across all timesteps
        for (size_t t = 1; t <= timesteps; ++t) {
            const auto& current_prices = price_path[t];

            // Update portfolio market prices
            auto& stocks = portfolio.get_stocks();
            for (size_t i = 0; i < stocks.size(); ++i) {
                stocks[i].update_price(current_prices[i]);
            }

            // Execute strategy step
            strat->on_step(t, current_prices, price_path, portfolio, res.stats);

            // Record portfolio valuation
            res.portfolio_values.push_back(portfolio.compute_value());
        }

        results.push_back(std::move(res));
    }

    return results;
}

void SimulationEngine::print_comparison_table(const std::vector<BacktestResult>& results) {
    std::cout << "\n" << std::string(105, '=') << "\n";
    std::cout << std::left
              << std::setw(25) << "Strategy"
              << std::right
              << std::setw(15) << "Final Value ($)"
              << std::setw(12) << "Return (%)"
              << std::setw(12) << "Daily Vol(%)"
              << std::setw(10) << "Sharpe"
              << std::setw(11) << "Max DD(%)"
              << std::setw(8)  << "Buys"
              << std::setw(8)  << "Sells"
              << std::setw(14) << "Volume ($)"
              << "\n";
    std::cout << std::string(105, '-') << "\n";

    for (const auto& r : results) {
        std::cout << std::left
                  << std::setw(25) << r.strategy_name
                  << std::right
                  << std::fixed
                  << std::setw(15) << std::setprecision(2) << r.final_value()
                  << std::setw(12) << std::setprecision(2) << r.total_return_pct()
                  << std::setw(12) << std::setprecision(3) << (r.daily_volatility() * 100.0)
                  << std::setw(10) << std::setprecision(2) << r.sharpe_ratio()
                  << std::setw(11) << std::setprecision(2) << r.max_drawdown_pct()
                  << std::setw(8)  << r.stats.buys
                  << std::setw(8)  << r.stats.sells
                  << std::setw(14) << std::setprecision(2) << r.stats.dollar_volume
                  << "\n";
    }
    std::cout << std::string(105, '=') << "\n\n";
}

} // namespace portfolio_engine
