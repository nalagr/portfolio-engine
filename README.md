# Portfolio Engine

A C++17 library for portfolio simulation, rebalancing, and mathematical optimization.

This project implements various portfolio strategies—from basic threshold rules to linear programming with the Simplex method and Markowitz mean-variance optimization. It includes a custom matrix library, a Tableau Simplex solver built from scratch, and a Geometric Brownian Motion market simulator.

---

## Features

- **Linear Algebra**: Matrix operations, determinants, inverses, and linear system solvers ($Ax = b$).
- **Simplex LP Solver**: Standard-form Tableau Simplex algorithm with Bland's rule to prevent cycling.
- **Trading & Rebalancing Strategies**:
  - Passive Buy & Hold benchmark.
  - Threshold trading (mean reversion triggers on reference prices).
  - Fixed-weight rebalancing ($Ax = b$).
  - Custom investor views (relative allocation constraints).
  - Dynamic Simplex optimization (rolling window expected return maximization).
  - Markowitz Mean-Variance QP and mixed-integer models (MILP/MIQP via optional Gurobi integration).
- **Market Simulator**: Multi-asset Geometric Brownian Motion price paths with covariance estimation.
- **Metrics & Export**: Tracking for daily volatility, Sharpe ratio, maximum drawdown, transaction counts, and CSV export.
- **Unit Tests**: Full test suite built on CTest.

---

## Project Structure

```
portfolio-engine/
├── CMakeLists.txt              # Build configuration
├── README.md                   # Project documentation
├── .gitignore
├── data/
│   └── portfolio.txt           # Sample initial portfolio (NVDA, AAPL, GOOG)
├── include/portfolio_engine/
│   ├── core/                   # Asset, Portfolio, Cash accounting
│   ├── linalg/                 # Matrix math and linear solvers
│   ├── optimization/           # Simplex solver and optional Gurobi solver
│   ├── simulation/             # GBM simulator and backtest runner
│   └── strategies/             # Strategy implementations
├── src/                        # Source files
├── examples/
│   └── run_backtest.cpp        # 1,000-step comparative simulation
└── tests/                      # Unit tests (Matrix, Simplex, Portfolio)
```

---

## How It Works

### 1. Asset Price Simulation
Asset prices follow discrete Geometric Brownian Motion with drift $\mu_i$ and volatility $\sigma_i$:
$$r_{i, t} = \mu_i + \sigma_i \cdot \epsilon_{i, t}, \quad \epsilon_{i, t} \sim \mathcal{N}(0, 1)$$
$$S_{i, t} = \max(0, \; S_{i, t-1} \cdot (1 + r_{i, t}))$$

### 2. Portfolio Valuation
For cash $C_t$ and shares $h_{i, t}$:
$$V_t = C_t + \sum_{i=1}^N h_{i, t} \cdot S_{i, t}$$

### 3. Strategies
- **Threshold Trading**: Buys 10 shares if price drops 5% below reference price; sells 10 shares if price rises 10% above reference price.
- **Fixed-Weight Rebalancing**: Solves $A_t x_t = b_t$ where $A_t$ is a diagonal matrix of asset prices and $b_t = w \cdot V_t$.
- **Investor Views**: Solves a system of equations matching target relative weights across assets.
- **Simplex LP**: Estimates trailing returns over 100 steps and solves a standard-form LP maximizing return under budget and position limits ($x_i \le 0.60$).
- **Markowitz QP (Optional)**: Maximizes risk-adjusted return $\mu^T x - \lambda x^T Q x$ where $Q$ is the trailing sample covariance matrix.

---

## Backtest Results

Simulated over 1,000 steps starting with \$100,000 cash and holdings in NVDA, AAPL, and GOOG:

| Strategy | Final Value ($) | Return (%) | Daily Vol (%) | Sharpe Ratio | Max DD (%) | Trades | Volume ($) |
|---|---:|---:|---:|---:|---:|---:|---:|
| **Buy & Hold** | \$307,681.83 | 87.6% | 0.61% | 1.69 | 9.36% | 0 | \$0.00 |
| **Threshold Trading** | \$358,510.11 | 118.6% | 0.64% | 1.98 | 7.14% | 205 | \$530,509.54 |
| **Fixed Weight (40/30/30)** | \$809,873.54 | 393.9% | 1.39% | 1.94 | 15.05% | 30 | \$718,731.38 |
| **Investor View** | \$918,500.44 | 460.2% | 1.60% | 1.84 | 17.94% | 30 | \$753,292.70 |
| **Simplex LP** | \$449,446.80 | 174.1% | 1.40% | 1.26 | 16.39% | 28 | \$2,037,569.97 |
| **Markowitz QP** *(Gurobi)* | \$477,185.98 | 191.0% | 1.40% | 1.32 | 16.39% | 28 | \$2,045,609.57 |
| **Cardinality ILP** *(Gurobi)* | \$404,068.06 | 146.4% | 1.05% | 1.45 | 10.37% | 27 | \$1,453,600.88 |
| **Semi-Continuous MILP** *(Gurobi)* | \$430,161.71 | 162.3% | 1.44% | 1.18 | 16.39% | 27 | \$2,473,453.33 |
| **Markowitz MIQP** *(Gurobi)* | \$430,161.71 | 162.3% | 1.44% | 1.18 | 16.39% | 27 | \$2,473,453.33 |

---

## Building and Running

### Requirements
- C++17 compiler (MSVC 2019+, GCC 9+, or Clang 10+)
- CMake 3.16+
- *(Optional)* Gurobi for QP/MILP models

### Build
Default build uses only standard C++ with no external dependencies:

```powershell
cmake -B build
cmake --build build --config Release
```

### Run Tests
```powershell
ctest --test-dir build -C Release --output-on-failure
```

### Run Simulation
```powershell
.\build\Release\run_backtest.exe
```

Outputs performance statistics and exports the full price paths and portfolio values to `portfolio_comparison.csv`.

### Enable Gurobi (Optional)
If Gurobi is installed and licensed:
```powershell
cmake -B build -DENABLE_GUROBI=ON
cmake --build build --config Release
```
