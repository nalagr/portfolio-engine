#include "portfolio_engine/core/asset.hpp"
#include <stdexcept>
#include <utility>

namespace portfolio_engine {

Asset::Asset(std::string symbol, int shares, double price, double reference_price)
    : symbol_(std::move(symbol)),
      shares_(shares),
      price_(price),
      reference_price_(reference_price >= 0.0 ? reference_price : price) {
    if (shares_ < 0) {
        throw std::invalid_argument("Initial shares cannot be negative: " + symbol_);
    }
    if (price_ < 0.0) {
        throw std::invalid_argument("Initial price cannot be negative: " + symbol_);
    }
}

double Asset::calculate_value() const noexcept {
    return static_cast<double>(shares_) * price_;
}

void Asset::update_price(double new_price) noexcept {
    price_ = (new_price < 0.0) ? 0.0 : new_price;
}

void Asset::buy_shares(int amount) {
    if (amount < 0) {
        throw std::invalid_argument("Buy amount cannot be negative");
    }
    shares_ += amount;
}

void Asset::sell_shares(int amount) {
    if (amount < 0) {
        throw std::invalid_argument("Sell amount cannot be negative");
    }
    if (amount > shares_) {
        throw std::runtime_error("Attempted to sell more shares than currently held for " + symbol_);
    }
    shares_ -= amount;
}

void Asset::set_reference_price(double new_reference_price) noexcept {
    reference_price_ = new_reference_price;
}

void Asset::set_shares(int new_shares) noexcept {
    shares_ = new_shares;
}

} // namespace portfolio_engine
