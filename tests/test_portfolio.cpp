#ifdef NDEBUG
#undef NDEBUG
#endif
#include "portfolio_engine/core/asset.hpp"
#include "portfolio_engine/core/portfolio.hpp"
#include "portfolio_engine/core/types.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace portfolio_engine;

void test_asset_operations() {
    std::cout << "[Test Portfolio] Running test_asset_operations...\n";
    Asset a("NVDA", 100, 50.0);
    assert(a.symbol() == "NVDA");
    assert(a.shares() == 100);
    assert(std::abs(a.price() - 50.0) < 1e-9);
    assert(std::abs(a.calculate_value() - 5000.0) < 1e-9);
    assert(std::abs(a.reference_price() - 50.0) < 1e-9);

    a.update_price(55.0);
    assert(std::abs(a.price() - 55.0) < 1e-9);
    assert(std::abs(a.calculate_value() - 5500.0) < 1e-9);

    // Verify bug fix: set_reference_price now updates reference price
    a.set_reference_price(55.0);
    assert(std::abs(a.reference_price() - 55.0) < 1e-9);

    a.buy_shares(20);
    assert(a.shares() == 120);

    a.sell_shares(50);
    assert(a.shares() == 70);

    // Exception on overselling
    bool caught_oversell = false;
    try {
        a.sell_shares(100);
    } catch (const std::runtime_error&) {
        caught_oversell = true;
    }
    assert(caught_oversell);
    std::cout << "  -> test_asset_operations PASSED\n";
}

void test_portfolio_trading() {
    std::cout << "[Test Portfolio] Running test_portfolio_trading...\n";
    std::vector<Asset> assets = {
        Asset("NVDA", 100, 50.0),
        Asset("AAPL", 200, 100.0)
    };
    Portfolio p(assets, 10000.0);
    // Value = 10000 (cash) + 100*50 (5000) + 200*100 (20000) = 35000
    assert(std::abs(p.compute_value() - 35000.0) < 1e-9);

    // Successful buy: buy 20 NVDA @ $50 = $1000 cost
    bool bought = p.buy_shares("NVDA", 20);
    assert(bought);
    assert(std::abs(p.cash() - 9000.0) < 1e-9);
    assert(p.find_asset("NVDA")->shares() == 120);
    assert(std::abs(p.compute_value() - 35000.0) < 1e-9); // Total value unchanged by transaction

    // Failed buy due to insufficient cash
    bool bought_too_much = p.buy_shares("AAPL", 500); // 500 * $100 = $50,000 > $9,000
    assert(!bought_too_much);
    assert(std::abs(p.cash() - 9000.0) < 1e-9); // Cash unchanged

    // Successful sell: sell 50 AAPL @ $100 = $5000 revenue
    bool sold = p.sell_shares("AAPL", 50);
    assert(sold);
    assert(std::abs(p.cash() - 14000.0) < 1e-9);
    assert(p.find_asset("AAPL")->shares() == 150);

    // Failed sell due to insufficient shares
    bool sold_too_much = p.sell_shares("AAPL", 1000);
    assert(!sold_too_much);
    std::cout << "  -> test_portfolio_trading PASSED\n";
}

void test_rebalancing_and_io() {
    std::cout << "[Test Portfolio] Running test_rebalancing_and_io...\n";
    // Stream loading test
    std::string mock_file = "NVDA 110 78.54\nAAPL 200 189.72\nGOOG 100 173.88\n";
    std::istringstream iss(mock_file);
    Portfolio p = Portfolio::load_from_stream(iss, 100000.0);

    // Initial value: 100000 + 110*78.54 (8639.40) + 200*189.72 (37944.00) + 100*173.88 (17388.00) = 163971.40
    double expected_val = 100000.0 + 8639.40 + 37944.00 + 17388.00;
    assert(std::abs(p.compute_value() - expected_val) < 1e-4);

    // Rebalance test: set NVDA target shares to 150 (+40 shares @ $78.54 = $3141.60)
    int buys = 0, sells = 0;
    double volume = 0.0;
    p.rebalance("NVDA", 150, buys, sells, volume);
    assert(p.find_asset("NVDA")->shares() == 150);
    assert(buys == 1);
    assert(sells == 0);
    assert(std::abs(volume - 3141.60) < 1e-4);

    // Rebalance test: set AAPL target shares to 150 (-50 shares @ $189.72 = $9486.00 proceeds)
    p.rebalance("AAPL", 150, buys, sells, volume);
    assert(p.find_asset("AAPL")->shares() == 150);
    assert(sells == 1);
    assert(std::abs(volume - (3141.60 + 9486.00)) < 1e-4);

    std::cout << "  -> test_rebalancing_and_io PASSED\n";
}

#include "portfolio_engine/simulation/backtest_result.hpp"
#include "portfolio_engine/strategies/threshold_strategy.hpp"

void test_threshold_trading_volume() {
    std::cout << "[Test Portfolio] Running test_threshold_trading_volume...\n";
    std::vector<Asset> assets = { Asset("NVDA", 100, 100.0, 100.0) };
    Portfolio p(assets, 10000.0);
    RebalanceStats stats;
    ThresholdTradingStrategy strat(10, 0.05, 0.10);

    // Drop price by 10% (100 -> 90) => triggers buy of 10 shares @ 90 = $900
    p.get_stocks()[0].update_price(90.0);
    strat.on_step(1, {90.0}, {{100.0}, {90.0}}, p, stats);

    assert(stats.buys == 1);
    assert(stats.sells == 0);
    assert(std::abs(stats.dollar_volume - 900.0) < 1e-4);
    assert(p.find_asset("NVDA")->shares() == 110);
    assert(std::abs(p.cash() - 9100.0) < 1e-4);

    // Rally price to 110 (ref is 90, +22%) => triggers sell of 10 shares @ 110 = $1100
    p.get_stocks()[0].update_price(110.0);
    strat.on_step(2, {110.0}, {{100.0}, {90.0}, {110.0}}, p, stats);

    assert(stats.buys == 1);
    assert(stats.sells == 1);
    assert(std::abs(stats.dollar_volume - (900.0 + 1100.0)) < 1e-4);
    assert(p.find_asset("NVDA")->shares() == 100);

    std::cout << "  -> test_threshold_trading_volume PASSED\n";
}

void test_rebalance_guards() {
    std::cout << "[Test Portfolio] Running test_rebalance_guards...\n";
    std::vector<Asset> assets = { Asset("TEST", 50, 100.0) };
    Portfolio p(assets, 500.0); // Only $500 cash
    int buys = 0, sells = 0;
    double vol = 0.0;

    // 1. Negative target shares clamp to 0 without throwing overselling exception
    p.rebalance("TEST", -10, buys, sells, vol);
    assert(p.find_asset("TEST")->shares() == 0);
    assert(sells == 1);
    assert(std::abs(p.cash() - (500.0 + 50 * 100.0)) < 1e-4); // Liquidated 50 shares

    // 2. Buy with insufficient cash: target 100 shares @ $100 = $10,000, but cash is $5,500
    // Should buy 55 shares, cash becomes $0, never negative
    p.rebalance("TEST", 100, buys, sells, vol);
    assert(p.find_asset("TEST")->shares() == 55);
    assert(p.cash() >= 0.0);
    assert(std::abs(p.cash()) < 1e-4);
    assert(buys == 1);

    std::cout << "  -> test_rebalance_guards PASSED\n";
}

void test_backtest_result_small_sample() {
    std::cout << "[Test Portfolio] Running test_backtest_result_small_sample...\n";
    BacktestResult res;
    // Empty
    assert(res.daily_volatility() == 0.0);
    assert(res.sharpe_ratio() == 0.0);

    // 1 point
    res.portfolio_values = {100.0};
    assert(res.daily_volatility() == 0.0);
    assert(res.sharpe_ratio() == 0.0);

    // 2 points (previously caused division by zero in sample volatility)
    res.portfolio_values = {100.0, 105.0};
    assert(res.daily_volatility() == 0.0);
    assert(res.sharpe_ratio() == 0.0);

    // 3 points (valid sample of 2 returns)
    res.portfolio_values = {100.0, 105.0, 102.0};
    assert(res.daily_volatility() > 0.0);

    std::cout << "  -> test_backtest_result_small_sample PASSED\n";
}

int main() {
    std::cout << "Running Portfolio Engine Portfolio Test Suite...\n";
    test_asset_operations();
    test_portfolio_trading();
    test_rebalancing_and_io();
    test_threshold_trading_volume();
    test_rebalance_guards();
    test_backtest_result_small_sample();
    std::cout << "ALL PORTFOLIO TESTS PASSED!\n";
    return 0;
}
