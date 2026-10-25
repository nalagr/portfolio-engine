#include "portfolio_engine/strategies/threshold_strategy.hpp"

namespace portfolio_engine {

ThresholdTradingStrategy::ThresholdTradingStrategy(int trade_size,
                                                   double buy_threshold_pct,
                                                   double sell_threshold_pct)
    : trade_size_(trade_size),
      buy_threshold_pct_(buy_threshold_pct),
      sell_threshold_pct_(sell_threshold_pct) {}

void ThresholdTradingStrategy::on_step(
    size_t /*t*/,
    const std::vector<double>& /*current_prices*/,
    const std::vector<std::vector<double>>& /*price_history*/,
    Portfolio& portfolio,
    RebalanceStats& stats) {

    auto& stocks = portfolio.get_stocks();
    for (auto& stock : stocks) {
        const std::string& ticker = stock.symbol();
        double ref_price = stock.reference_price();
        double price = stock.price();

        if (price <= (1.0 - buy_threshold_pct_) * ref_price) {
            if (portfolio.buy_shares(ticker, trade_size_)) {
                stock.set_reference_price(price);
                stats.buys++;
                stats.dollar_volume += static_cast<double>(trade_size_) * price;
                trade_counts_[ticker]++;
            }
        } else if (price >= (1.0 + sell_threshold_pct_) * ref_price) {
            if (portfolio.sell_shares(ticker, trade_size_)) {
                stock.set_reference_price(price);
                stats.sells++;
                stats.dollar_volume += static_cast<double>(trade_size_) * price;
                trade_counts_[ticker]++;
            }
        }
    }
}

void ThresholdTradingStrategy::reset() {
    trade_counts_.clear();
}

} // namespace portfolio_engine
