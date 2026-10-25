#include "bs.hpp"

#include <cmath>
#include <stdexcept>

namespace {

constexpr double kInvSqrt2 = 0.70710678118654752440;   // 1 / sqrt(2)
constexpr double kInvSqrt2Pi = 0.39894228040143267794; // 1 / sqrt(2 * pi)

// Standard normal CDF. erfc keeps precision in the far left tail,
// where 1 + erf(x) would suffer from cancellation.
double norm_cdf(double x) {
    return 0.5 * std::erfc(-x * kInvSqrt2);
}

// Standard normal PDF.
double norm_pdf(double x) {
    return kInvSqrt2Pi * std::exp(-0.5 * x * x);
}

void require_positive(double value, const char* name) {
    if (!std::isfinite(value) || value <= 0.0) {
        throw std::invalid_argument(std::string(name) + " must be a positive finite number");
    }
}

const BSParams& validated(const BSParams& p) {
    require_positive(p.S, "S");
    require_positive(p.K, "K");
    require_positive(p.sigma, "sigma");
    require_positive(p.T, "T");
    if (!std::isfinite(p.r)) {
        throw std::invalid_argument("r must be a finite number");
    }
    return p;
}

} // namespace

// ---------------------------------------------------------------------------
// BlackScholes
// ---------------------------------------------------------------------------

BlackScholes::BlackScholes(const BSParams& p)
    : p_(validated(p)),
      sqrt_T_(std::sqrt(p.T)),
      discount_(std::exp(-p.r * p.T)),
      d1_((std::log(p.S / p.K) + (p.r + 0.5 * p.sigma * p.sigma) * p.T) / (p.sigma * sqrt_T_)),
      d2_(d1_ - p.sigma * sqrt_T_) {}

double BlackScholes::call_price() const {
    return p_.S * norm_cdf(d1_) - p_.K * discount_ * norm_cdf(d2_);
}

double BlackScholes::put_price() const {
    return p_.K * discount_ * norm_cdf(-d2_) - p_.S * norm_cdf(-d1_);
}

double BlackScholes::delta_call() const {
    return norm_cdf(d1_);
}

double BlackScholes::delta_put() const {
    return norm_cdf(d1_) - 1.0;
}

double BlackScholes::gamma() const {
    return norm_pdf(d1_) / (p_.S * p_.sigma * sqrt_T_);
}

double BlackScholes::vega() const {
    return p_.S * norm_pdf(d1_) * sqrt_T_;
}

double BlackScholes::theta_call() const {
    const double time_decay = -p_.S * p_.sigma * norm_pdf(d1_) / (2.0 * sqrt_T_);
    return time_decay - p_.r * p_.K * discount_ * norm_cdf(d2_);
}

double BlackScholes::theta_put() const {
    const double time_decay = -p_.S * p_.sigma * norm_pdf(d1_) / (2.0 * sqrt_T_);
    return time_decay + p_.r * p_.K * discount_ * norm_cdf(-d2_);
}

// ---------------------------------------------------------------------------
// Options
// ---------------------------------------------------------------------------

CallOption::CallOption(const BlackScholes& model) : model_(model) {}

Quote CallOption::quote() const {
    Quote q;
    q.price = model_.call_price();
    q.delta = model_.delta_call();
    q.gamma = model_.gamma();
    q.vega = model_.vega();
    q.theta = model_.theta_call();
    return q;
}

PutOption::PutOption(const BlackScholes& model) : model_(model) {}

Quote PutOption::quote() const {
    Quote q;
    q.price = model_.put_price();
    q.delta = model_.delta_put();
    q.gamma = model_.gamma();
    q.vega = model_.vega();
    q.theta = model_.theta_put();
    return q;
}

std::unique_ptr<Option> make_option(const std::string& style, const BlackScholes& model) {
    if (style == "call") {
        return std::make_unique<CallOption>(model);
    }
    if (style == "put") {
        return std::make_unique<PutOption>(model);
    }
    throw std::invalid_argument("unknown option style '" + style + "' (expected 'call' or 'put')");
}
