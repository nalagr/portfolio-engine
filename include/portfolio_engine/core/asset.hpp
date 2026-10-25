#pragma once

#include <string>

namespace portfolio_engine {

/**
 * @brief Represents a single financial asset held within a portfolio.
 */
class Asset {
public:
    Asset(std::string symbol, int shares, double price, double reference_price = -1.0);

    // Core accessors
    const std::string& symbol() const noexcept { return symbol_; }
    int shares() const noexcept { return shares_; }
    double price() const noexcept { return price_; }
    double reference_price() const noexcept { return reference_price_; }

    // Compatibility aliases with homework API
    std::string get_name() const { return symbol_; }
    int get_shares() const noexcept { return shares_; }
    double get_price() const noexcept { return price_; }
    double get_reference_price() const noexcept { return reference_price_; }

    // Mutators & Operations
    double calculate_value() const noexcept;
    void update_price(double new_price) noexcept;
    void buy_shares(int amount);
    void sell_shares(int amount);
    void set_reference_price(double new_reference_price) noexcept;
    void set_shares(int new_shares) noexcept;

private:
    std::string symbol_;
    int shares_{0};
    double price_{0.0};
    double reference_price_{0.0};
};

} // namespace portfolio_engine
