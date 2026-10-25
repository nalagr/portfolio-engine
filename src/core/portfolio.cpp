#include "portfolio_engine/core/portfolio.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace portfolio_engine {

Portfolio::Portfolio(std::vector<Asset> starting_assets, double starting_cash)
    : assets_(std::move(starting_assets)),
      cash_(starting_cash) {
    if (cash_ < 0.0) {
        throw std::invalid_argument("Starting cash cannot be negative");
    }
    compute_value();
}

double Portfolio::compute_value() noexcept {
    double total = cash_;
    for (const auto& asset : assets_) {
        total += asset.calculate_value();
    }
    portfolio_value_ = total;
    return portfolio_value_;
}

Asset* Portfolio::find_asset(const std::string& symbol) noexcept {
    for (auto& asset : assets_) {
        if (asset.symbol() == symbol) {
            return &asset;
        }
    }
    return nullptr;
}

const Asset* Portfolio::find_asset(const std::string& symbol) const noexcept {
    for (const auto& asset : assets_) {
        if (asset.symbol() == symbol) {
            return &asset;
        }
    }
    return nullptr;
}

int Portfolio::asset_index(const std::string& symbol) const noexcept {
    for (size_t i = 0; i < assets_.size(); ++i) {
        if (assets_[i].symbol() == symbol) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool Portfolio::buy_shares(const std::string& symbol, int amount) {
    if (amount <= 0) return false;
    Asset* asset = find_asset(symbol);
    if (!asset) return false;

    double cost = static_cast<double>(amount) * asset->price();
    if (cash_ >= cost) {
        cash_ -= cost;
        asset->buy_shares(amount);
        compute_value();
        return true;
    }
    return false;
}

bool Portfolio::sell_shares(const std::string& symbol, int amount) {
    if (amount <= 0) return false;
    Asset* asset = find_asset(symbol);
    if (!asset) return false;

    if (asset->shares() >= amount) {
        asset->sell_shares(amount);
        cash_ += static_cast<double>(amount) * asset->price();
        compute_value();
        return true;
    }
    return false;
}

void Portfolio::rebalance(const std::string& symbol, int target_shares,
                          int& buys, int& sells, double& dollar_volume) {
    Asset* asset = find_asset(symbol);
    if (!asset) return;

    if (target_shares < 0) {
        target_shares = 0;
    }

    int diff = target_shares - asset->shares();
    if (diff > 0) {
        double cost = static_cast<double>(diff) * asset->price();
        if (cost > cash_) {
            // Cannot buy full diff shares without going into debt; buy what cash permits
            int affordable = (asset->price() > 0.0) ? static_cast<int>(cash_ / asset->price()) : 0;
            if (affordable > 0) {
                double actual_cost = static_cast<double>(affordable) * asset->price();
                asset->buy_shares(affordable);
                cash_ -= actual_cost;
                buys++;
                dollar_volume += actual_cost;
            }
        } else {
            asset->buy_shares(diff);
            cash_ -= cost;
            buys++;
            dollar_volume += cost;
        }
    } else if (diff < 0) {
        int sell_amount = -diff;
        if (sell_amount > asset->shares()) {
            sell_amount = asset->shares();
        }
        if (sell_amount > 0) {
            double proceeds = static_cast<double>(sell_amount) * asset->price();
            asset->sell_shares(sell_amount);
            cash_ += proceeds;
            sells++;
            dollar_volume += proceeds;
        }
    }
    compute_value();
}

void Portfolio::rebalance_weights(const std::vector<double>& weights,
                                 const std::vector<double>& prices,
                                 int& buys, int& sells, double& dollar_volume) {
    if (weights.size() != assets_.size() || prices.size() != assets_.size()) {
        throw std::invalid_argument("Dimension mismatch in rebalance_weights");
    }

    double current_val = compute_value();

    // Pass 1: Sell positions first to generate liquid cash
    for (size_t i = 0; i < assets_.size(); ++i) {
        if (prices[i] <= 0.0) continue;
        int target_shares = std::max(0, static_cast<int>((current_val * weights[i]) / prices[i]));
        if (target_shares < assets_[i].shares()) {
            rebalance(assets_[i].symbol(), target_shares, buys, sells, dollar_volume);
        }
    }

    // Pass 2: Buy positions to meet target allocation
    for (size_t i = 0; i < assets_.size(); ++i) {
        if (prices[i] <= 0.0) continue;
        int target_shares = std::max(0, static_cast<int>((current_val * weights[i]) / prices[i]));
        if (target_shares > assets_[i].shares()) {
            rebalance(assets_[i].symbol(), target_shares, buys, sells, dollar_volume);
        }
    }
}

void Portfolio::portfolio_statistics() const {
    std::cout << "cash: $" << std::fixed << std::setprecision(2) << cash_ << "\n";
    std::cout << "holdings: \n";
    for (const auto& stock : assets_) {
        std::cout << "  " << stock.symbol() << ": " << stock.shares()
                  << " shares at $" << stock.price() << " (total value: $"
                  << stock.calculate_value() << ")\n";
    }
    std::cout << "total portfolio value: $" << portfolio_value_ << "\n";
}

Portfolio Portfolio::load_from_stream(std::istream& is, double starting_cash) {
    std::vector<Asset> loaded;
    std::string name;
    int shares = 0;
    double price = 0.0;
    while (is >> name >> shares >> price) {
        loaded.emplace_back(name, shares, price);
    }
    return Portfolio(std::move(loaded), starting_cash);
}

Portfolio Portfolio::load_from_file(const std::string& filename, double starting_cash) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open portfolio file: " + filename);
    }
    return load_from_stream(file, starting_cash);
}

std::vector<Asset> read_portfolio(const std::string& filename) {
    std::vector<Asset> stocks;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open portfolio file: " + filename);
    }
    std::string name;
    int num = 0;
    double price = 0.0;
    while (file >> name >> num >> price) {
        stocks.emplace_back(name, num, price);
    }
    return stocks;
}

} // namespace portfolio_engine
