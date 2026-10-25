# Black–Scholes Option Pricer (C++)

[![CI](https://github.com/SlavaPyt/black-scholes-fdm/actions/workflows/ci.yml/badge.svg)](https://github.com/SlavaPyt/black-scholes-fdm/actions/workflows/ci.yml)

This project implements the classical Black–Scholes model for pricing European options in modern C++ (C++17).

The engine computes option prices and the main Greeks. It is available as a command-line tool, which can also export results over a grid of spot prices to CSV, and as a Python module, so experiments and plots can be written in Python while all computation runs in C++.

The goal of the project is to explore how quantitative finance models are implemented in real software, while practicing clean C++ architecture and project structure.

## Features

- Pricing of European call and put options
- Greeks: delta, gamma, vega, theta
- Input validation with clear error messages
- Command-line interface with a grid mode and CSV export
- Python bindings via pybind11
- Python plotting script for visualization and analysis
- Unit tests for the C++ core, the CLI and the Python module, run on every push by GitHub Actions

The code uses an object-oriented design: a generic `Option` interface is implemented by the `CallOption` and `PutOption` classes, which are created by the `make_option` factory.

## Mathematical model

The Black–Scholes model assumes that the underlying asset follows geometric Brownian motion with constant volatility σ and a constant risk-free rate r, and that the asset pays no dividends.

The prices of a European call and put are

```
C = S·N(d1) − K·e^(−rT)·N(d2)
P = K·e^(−rT)·N(−d2) − S·N(−d1)

d1 = ( ln(S/K) + (r + σ²/2)·T ) / ( σ·√T )
d2 = d1 − σ·√T
```

where N is the standard normal CDF. Prices satisfy put–call parity: C − P = S − K·e^(−rT).

Greeks (φ is the standard normal density):

| Greek | Call | Put |
|-------|------|-----|
| delta | N(d1) | N(d1) − 1 |
| gamma | φ(d1) / (S·σ·√T) | same as call |
| vega  | S·φ(d1)·√T | same as call |
| theta | −S·σ·φ(d1) / (2√T) − r·K·e^(−rT)·N(d2) | −S·σ·φ(d1) / (2√T) + r·K·e^(−rT)·N(−d2) |

Units: vega is the change in price for a change of 1.00 in σ (divide by 100 for "per 1%"), theta is the change in price per year (divide by 365 for "per day").

## Project structure

```
black-scholes-fdm/
├── include/
│   └── bs.hpp                 # model, option classes, factory
├── src/
│   ├── bs.cpp                 # pricing formulas
│   └── main.cpp               # command-line interface
├── python_bindings/
│   └── bindings.cpp           # pybind11 module bs_engine
├── python_tools/
│   └── plot_from_engine.py    # price vs spot plot
├── tests/
│   ├── test_bs.cpp            # C++ unit tests
│   └── test_bindings.py       # Python module tests
├── images/
├── .github/workflows/ci.yml   # GitHub Actions
└── CMakeLists.txt
```

## Build

Requirements: CMake 3.16+, a C++17 compiler and, for the Python module, Python 3.8+ with pybind11.

```bash
python -m pip install pybind11      # only needed for the Python module
cmake -S . -B build
cmake --build build
```

This produces:

1. `black_scholes` – the command-line tool
2. `bs_engine` – the Python extension module
3. `bs_tests` – the C++ unit tests

If pybind11 is not installed, CMake prints a warning and builds everything except the Python module. You can also turn it off explicitly with `-DBS_BUILD_PYTHON=OFF`.

## Command-line usage

Price a single option:

```bash
./build/black_scholes --S 100 --K 100 --r 0.05 --sigma 0.2 --T 1 --style call
```

```
price=10.4506
delta=0.636831
gamma=0.018762
vega=37.524
theta=-6.41403
```

| Parameter | Meaning | Default |
|-----------|---------|---------|
| `--S` | spot price of the underlying | 100 |
| `--K` | strike price | 100 |
| `--r` | risk-free rate, per year | 0 |
| `--sigma` | volatility, per year | 0.2 |
| `--T` | time to maturity, in years | 1 |
| `--style` | `call` or `put` | `call` |

Run `./build/black_scholes --help` for the full list of options. Invalid input (a missing value, a non-numeric value, σ ≤ 0, an unknown option) produces an error message and a non-zero exit code.

### Grid mode

Price the option on N evenly spaced spot prices between START and END and save the results to CSV:

```bash
./build/black_scholes --grid-S 50 150 101 --style call --out prices.csv
```

The CSV has the columns `S,price,delta,gamma,vega,theta` and can be used for plotting or further analysis.

## Python usage

After building, the module can be imported from the `build/` directory:

```python
import sys
sys.path.insert(0, "build")

import bs_engine

params = bs_engine.BSParams(S=100.0, K=100.0, r=0.05, sigma=0.2, T=1.0)

call = bs_engine.quote("call", params)
print(call)        # Quote(price=10.4506, delta=0.636831, gamma=0.018762, vega=37.524, theta=-6.41403)

put = bs_engine.put_quote(params)
print(put.price)   # 5.573526022256971
```

Invalid parameters raise `ValueError`.

## Plotting from Python

The plotting workflow keeps computation in C++ and visualization in Python:

1. Python creates an array of spot prices S
2. For each S it calls the C++ engine
3. The results are collected in Python lists
4. matplotlib builds the plot

```bash
python python_tools/plot_from_engine.py                          # call price, shown on screen
python python_tools/plot_from_engine.py --style put --sigma 0.3 --save put.png
```

### Example plot

![Call price vs S](images/CallPrice_Vs_S.png)

## Tests

```bash
ctest --test-dir build --output-on-failure    # C++ unit tests and CLI checks
python -m unittest discover -s tests -v       # Python module tests
```

The tests check prices and Greeks against reference values (including the example from Hull, *Options, Futures, and Other Derivatives*), put–call parity, no-arbitrage bounds, Greeks against finite-difference derivatives of the price, and the handling of invalid input. GitHub Actions runs all of them on Linux and macOS for every push and pull request.

## Roadmap

- plotting scripts for all Greeks
- comparison of call and put curves
- parameter sweeps over volatility and maturity
- implied volatility solver
- binomial tree model
- Monte Carlo pricing
- finite-difference solver for the Black–Scholes PDE (the "fdm" in the repository name)
- comparison of analytical and numerical methods

## Why this project

The main idea is to build a small research tool with a numerical core in C++ and a convenient experimentation layer in Python, and to extend it later with PDE and finite-difference methods.

## Author

Built as a learning project in quantitative finance.

Vyacheslav  
ETU "LETI"
