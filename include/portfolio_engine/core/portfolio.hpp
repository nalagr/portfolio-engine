#pragma once

#include "portfolio_engine/core/asset.hpp"
#include <iosfwd>
#include <string>
#include <vector>

namespace portfolio_engine {

/**
 * @brief Manages a collection of assets and liquid cash balance.
 */
class Portfolio {
public:
    Portfolio(std::vector<Asset> starting_assets, double starting_cash);

    // Valuations
    double compute_value() noexcept;
    double cash() const noexcept { return cash_; }
    double value() const noexcept { return portfolio_value_; }

    // Compatibility getters
    double get_cash() const noexcept { return cash_; }
    double get_value() const noexcept { return portfolio_value_; }

    // Asset accessors
    const std::vector<Asset>& assets() const noexcept { return assets_; }
    std::vector<Asset>& assets() noexcept { return assets_; }
    std::vector<Asset>& get_stocks() noexcept { return assets_; }
    const std::vector<Asset>& get_stocks() const noexcept { return assets_; }

    Asset* find_asset(const std::string& symbol) noexcept;
    const Asset* find_asset(const std::string& symbol) const noexcept;
    int asset_index(const std::string& symbol) const noexcept;

    // Trading primitives
    bool buy_shares(const std::string& symbol, int amount);
    bool sell_shares(const std::string& symbol, int amount);

    // Rebalancing mechanisms
    void rebalance(const std::string& symbol, int target_shares, int& buys, int& sells, double& dollar_volume);
    void rebalancing(const std::string& ticker, int shares, int& buys, int& sells, double& dollar_volume) {
        rebalance(ticker, shares, buys, sells, dollar_volume);
    }
    void rebalance_weights(const std::vector<double>& weights, const std::vector<double>& prices,
                           int& buys, int& sells, double& dollar_volume);

    // Diagnostics & Serialization
    void portfolio_statistics() const;
    static Portfolio load_from_file(const std::string& filename, double starting_cash);
    static Portfolio load_from_stream(std::istream& is, double starting_cash);

private:
    std::vector<Asset> assets_;
    double cash_{0.0};
    double portfolio_value_{0.0};
};

// Global file parser utility matching original homework signature
std::vector<Asset> read_portfolio(const std::string& filename);

} // namespace portfolio_engine
