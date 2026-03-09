# Black–Scholes Option Pricer (C++)

This project implements the classical Black–Scholes model for pricing European options using modern C++.

The program computes option prices and several key Greeks. It also supports command-line parameters and a grid mode that exports pricing results to CSV for further analysis.

The goal of the project is to explore how quantitative finance models are implemented in real software systems while practicing clean C++ architecture and project structure.


## What the program can do

The implementation currently supports:

- pricing European call and put options
- computing Greeks:
  - delta
  - gamma
  - vega
- command-line interface
- grid mode for generating price data
- CSV export for further analysis

The code uses an object-oriented design where a generic `Option` interface is implemented by `CallOption` and `PutOption` classes.

The project is built with CMake.


## Mathematical model

The Black–Scholes model assumes that the underlying asset follows a lognormal process with constant volatility and interest rate.

The price of a European call option is given by:

C = S N(d1) − K e^(−rT) N(d2)

where

d1 = ( ln(S/K) + (r + σ² / 2) T ) / ( σ √T )

d2 = d1 − σ √T


## Project structure

black-scholes-fdm/

include/
    bs.hpp

src/
    bs.cpp
    main.cpp

CMakeLists.txt


## Build

Clone the repository and build using CMake.

git clone https://github.com/SlavaPyt/black-scholes-fdm.git
cd black-scholes-fdm

mkdir build
cd build

cmake ..
make


## Usage

Example calculation:

./black_scholes --S 100 --K 100 --r 0.05 --sigma 0.2 --T 1 --style call

Parameters:

S – underlying price  
K – strike price  
r – risk-free rate  
sigma – volatility  
T – time to maturity  
style – call or put


## Grid mode

Example:

./black_scholes --grid-S 50 150 101 --style call --out prices.csv

The program generates a CSV file that can be used for plotting price curves or further analysis.


## Future improvements

Possible extensions:

- theta calculation
- implied volatility solver
- binomial tree model
- Monte Carlo pricing
- volatility surfaces


## Author

Vyacheslav  
ETU "LETI"
