#pragma once

#include <memory>
#include <string>

// Input parameters of the Black–Scholes model (data transfer object).
struct BSParams {
    double S = 0.0;      // spot price of the underlying
    double K = 0.0;      // strike price
    double r = 0.0;      // continuously compounded risk-free rate, per year
    double sigma = 0.0;  // volatility, per year
    double T = 0.0;      // time to maturity, in years
};

// Price and Greeks of a single option.
//   vega  — change in price per 1.00 change in sigma (divide by 100 for "per 1%")
//   theta — change in price per year of passing time (divide by 365 for "per day")
struct Quote {
    double price = 0.0;
    double delta = 0.0;
    double gamma = 0.0;
    double vega = 0.0;
    double theta = 0.0;
};

// Closed-form Black–Scholes pricer for European options on a non-dividend-paying asset.
// The constructor validates the parameters and throws std::invalid_argument
// if S, K, sigma or T are not strictly positive or any value is not finite.
class BlackScholes {
public:
    explicit BlackScholes(const BSParams& p);

    const BSParams& params() const { return p_; }

    double call_price() const;
    double put_price() const;

    double delta_call() const;
    double delta_put() const;

    double theta_call() const;
    double theta_put() const;

    // Gamma and vega are the same for calls and puts.
    double gamma() const;
    double vega() const;

private:
    BSParams p_;
    double sqrt_T_;
    double discount_;  // e^(-rT)
    double d1_;
    double d2_;
};

// Common interface for option types priced by a BlackScholes model.
class Option {
public:
    virtual ~Option() = default;
    virtual Quote quote() const = 0;
};

// Options keep their own copy of the model, so they stay valid
// even if the model they were created from goes out of scope.
class CallOption : public Option {
public:
    explicit CallOption(const BlackScholes& model);
    Quote quote() const override;

private:
    BlackScholes model_;
};

class PutOption : public Option {
public:
    explicit PutOption(const BlackScholes& model);
    Quote quote() const override;

private:
    BlackScholes model_;
};

// Factory: style must be "call" or "put", otherwise throws std::invalid_argument.
std::unique_ptr<Option> make_option(const std::string& style, const BlackScholes& model);
