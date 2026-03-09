# Black–Scholes Option Pricer (C++)

Implementation of the Black–Scholes model for pricing European options written in modern C++.

The project computes option price and several Greeks and supports both single calculations and grid simulations with CSV export.

This project was built as a learning exercise in:

- quantitative finance
- numerical models
- modern C++ design
- object-oriented architecture


## Features

- European Call / Put option pricing
- Greeks calculation:
  - Delta
  - Gamma
  - Vega
- Command line interface
- Grid mode for generating price surfaces
- CSV export for further analysis
- Object-oriented architecture

Option
 ├── CallOption
 └── PutOption

- Built with CMake


## Mathematical Model

The Black–Scholes model prices European options assuming:

- lognormal asset dynamics
- constant volatility
- continuous compounding
- no arbitrage

The option price is computed using:

C = S N(d1) − K e^(−rT) N(d2)

where

d1 = ( ln(S/K) + (r + σ²/2)T ) / ( σ√T )

d2 = d1 − σ√T


## Project Structure

black-scholes-fdm

include/
  bs.hpp

src/
  bs.cpp
  main.cpp

CMakeLists.txt
README.md


## Build

Clone the repository and build using CMake.

git clone https://github.com/SlavaPyt/black-scholes-fdm.git
cd black-scholes-fdm

mkdir build
cd build

cmake ..
make


## Usage

Run the executable with parameters:

./black_scholes --S 100 --K 100 --r 0.05 --sigma 0.2 --T 1 --style call

Parameters:

S      — underlying price  
K      — strike price  
r      — risk-free interest rate  
sigma  — volatility  
T      — time to maturity  
style  — call / put


## Grid Mode

Generate prices for a range of underlying prices:

./black_scholes --grid-S 50 150 101 --style call --out prices.csv

This produces a CSV file suitable for plotting or further analysis.


## Example Output

price = 10.45  
delta = 0.63  
gamma = 0.018  
vega  = 37.52  


## Planned Improvements

Future extensions of the project may include:

- Theta
- Implied volatility solver
- CRR binomial tree model
- Monte Carlo pricing
- Volatility surfaces


## Motivation

The goal of this project is to better understand how financial models are implemented in real software systems.

It also serves as practice in:

- C++
- numerical finance
- project structure
- Git / GitHub workflow


## Author

Vyacheslav  
ETU "LETI"
