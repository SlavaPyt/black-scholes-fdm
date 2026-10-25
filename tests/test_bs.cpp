// Unit tests for the Black–Scholes core.
// A tiny self-contained harness keeps the project free of external test frameworks.

#include "bs.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

int g_checks = 0;
int g_failures = 0;

void check(bool ok, const std::string& what) {
    ++g_checks;
    if (!ok) {
        ++g_failures;
        std::cerr << "FAIL: " << what << "\n";
    }
}

void check_close(double actual, double expected, double tol, const std::string& what) {
    const bool ok = std::abs(actual - expected) <= tol;
    if (!ok) {
        std::cerr.precision(12);
        std::cerr << "  expected " << expected << ", got " << actual << " (tol " << tol << ")\n";
    }
    check(ok, what);
}

template <typename F>
void check_throws(F&& f, const std::string& what) {
    bool threw = false;
    try {
        f();
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    check(threw, what + " throws std::invalid_argument");
}

std::string describe(const BSParams& p) {
    return "S=" + std::to_string(p.S) + " K=" + std::to_string(p.K) + " r=" + std::to_string(p.r) +
           " sigma=" + std::to_string(p.sigma) + " T=" + std::to_string(p.T);
}

// A spread of in-, at- and out-of-the-money cases, short and long maturities,
// zero and negative rates.
const std::vector<BSParams> kCases = {
    {100.0, 100.0, 0.05, 0.20, 1.00},
    {42.0, 40.0, 0.10, 0.20, 0.50},
    {80.0, 100.0, 0.03, 0.35, 2.00},
    {120.0, 100.0, 0.00, 0.15, 0.25},
    {100.0, 110.0, -0.01, 0.50, 0.10},
    {50.0, 45.0, 0.07, 0.10, 5.00},
};

// --------------------------------------------------------------------------

void test_reference_values() {
    // Values computed independently from the closed-form formulas.
    const BlackScholes atm({100.0, 100.0, 0.05, 0.2, 1.0});
    const double tol = 1e-9;
    check_close(atm.call_price(), 10.450583572186, tol, "ATM call price");
    check_close(atm.put_price(), 5.573526022257, tol, "ATM put price");
    check_close(atm.delta_call(), 0.636830651176, tol, "ATM call delta");
    check_close(atm.delta_put(), -0.363169348824, tol, "ATM put delta");
    check_close(atm.gamma(), 0.018762017346, tol, "ATM gamma");
    check_close(atm.vega(), 37.524034691694, tol, "ATM vega");
    check_close(atm.theta_call(), -6.414027546438, tol, "ATM call theta");
    check_close(atm.theta_put(), -1.657880423935, tol, "ATM put theta");

    // Hull, "Options, Futures, and Other Derivatives", Example 15.6: c = 4.76, p = 0.81
    const BlackScholes hull({42.0, 40.0, 0.10, 0.2, 0.5});
    check_close(hull.call_price(), 4.759422392872, tol, "Hull example call price");
    check_close(hull.put_price(), 0.808599372900, tol, "Hull example put price");
}

void test_put_call_parity() {
    // C - P = S - K e^(-rT)
    for (const BSParams& p : kCases) {
        const BlackScholes m(p);
        const double lhs = m.call_price() - m.put_price();
        const double rhs = p.S - p.K * std::exp(-p.r * p.T);
        check_close(lhs, rhs, 1e-10, "put-call parity, " + describe(p));
        check_close(m.delta_call() - m.delta_put(), 1.0, 1e-12, "delta parity, " + describe(p));
    }
}

void test_no_arbitrage_bounds() {
    for (const BSParams& p : kCases) {
        const BlackScholes m(p);
        const double discounted_K = p.K * std::exp(-p.r * p.T);
        const double c = m.call_price();
        const double put = m.put_price();
        check(c >= std::max(p.S - discounted_K, 0.0) - 1e-12 && c <= p.S, "call bounds, " + describe(p));
        check(put >= std::max(discounted_K - p.S, 0.0) - 1e-12 && put <= discounted_K,
              "put bounds, " + describe(p));
        check(m.gamma() > 0.0 && m.vega() > 0.0, "gamma and vega positive, " + describe(p));
    }
}

// Greeks must match numerical derivatives of the price (central differences).
void test_greeks_match_finite_differences() {
    for (const BSParams& p : kCases) {
        const BlackScholes m(p);
        const std::string where = ", " + describe(p);

        auto priced = [](BSParams q) { return BlackScholes(q); };

        const double hS = 1e-4 * p.S;
        BSParams up = p, down = p;
        up.S += hS;
        down.S -= hS;
        const BlackScholes mu = priced(up), md = priced(down);
        check_close(m.delta_call(), (mu.call_price() - md.call_price()) / (2 * hS), 1e-6, "call delta" + where);
        check_close(m.delta_put(), (mu.put_price() - md.put_price()) / (2 * hS), 1e-6, "put delta" + where);
        check_close(m.gamma(), (mu.call_price() - 2 * m.call_price() + md.call_price()) / (hS * hS), 1e-4,
                    "gamma" + where);

        const double hSig = 1e-5;
        BSParams vu = p, vd = p;
        vu.sigma += hSig;
        vd.sigma -= hSig;
        check_close(m.vega(), (priced(vu).call_price() - priced(vd).call_price()) / (2 * hSig), 1e-5,
                    "vega" + where);

        // theta = dV/dt = -dV/dT
        const double hT = 1e-6;
        BSParams tu = p, td = p;
        tu.T += hT;
        td.T -= hT;
        check_close(m.theta_call(), -(priced(tu).call_price() - priced(td).call_price()) / (2 * hT), 1e-4,
                    "call theta" + where);
        check_close(m.theta_put(), -(priced(tu).put_price() - priced(td).put_price()) / (2 * hT), 1e-4,
                    "put theta" + where);
    }
}

void test_option_factory() {
    const BlackScholes m({100.0, 100.0, 0.05, 0.2, 1.0});

    const Quote call = make_option("call", m)->quote();
    check_close(call.price, m.call_price(), 0.0, "factory call price");
    check_close(call.delta, m.delta_call(), 0.0, "factory call delta");
    check_close(call.theta, m.theta_call(), 0.0, "factory call theta");

    const Quote put = make_option("put", m)->quote();
    check_close(put.price, m.put_price(), 0.0, "factory put price");
    check_close(put.delta, m.delta_put(), 0.0, "factory put delta");
    check_close(put.theta, m.theta_put(), 0.0, "factory put theta");

    check_throws([&] { make_option("straddle", m); }, "unknown style");
}

void test_option_outlives_model() {
    // The model is a temporary here; the option must keep its own copy.
    const auto option = make_option("call", BlackScholes({100.0, 100.0, 0.05, 0.2, 1.0}));
    check_close(option->quote().price, 10.450583572186, 1e-9, "option outlives its model");
}

void test_invalid_parameters() {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();

    check_throws([] { BlackScholes({0.0, 100.0, 0.05, 0.2, 1.0}); }, "S = 0");
    check_throws([] { BlackScholes({-1.0, 100.0, 0.05, 0.2, 1.0}); }, "S < 0");
    check_throws([] { BlackScholes({100.0, 0.0, 0.05, 0.2, 1.0}); }, "K = 0");
    check_throws([] { BlackScholes({100.0, 100.0, 0.05, 0.0, 1.0}); }, "sigma = 0");
    check_throws([] { BlackScholes({100.0, 100.0, 0.05, -0.2, 1.0}); }, "sigma < 0");
    check_throws([] { BlackScholes({100.0, 100.0, 0.05, 0.2, 0.0}); }, "T = 0");
    check_throws([&] { BlackScholes({nan, 100.0, 0.05, 0.2, 1.0}); }, "S = NaN");
    check_throws([&] { BlackScholes({100.0, 100.0, inf, 0.2, 1.0}); }, "r = inf");

    bool negative_rate_ok = true;
    try {
        BlackScholes({100.0, 100.0, -0.02, 0.2, 1.0});
    } catch (const std::exception&) {
        negative_rate_ok = false;
    }
    check(negative_rate_ok, "negative interest rate is allowed");
}

} // namespace

int main() {
    test_reference_values();
    test_put_call_parity();
    test_no_arbitrage_bounds();
    test_greeks_match_finite_differences();
    test_option_factory();
    test_option_outlives_model();
    test_invalid_parameters();

    if (g_failures > 0) {
        std::cerr << g_failures << " of " << g_checks << " checks failed\n";
        return 1;
    }
    std::cout << "All " << g_checks << " checks passed\n";
    return 0;
}
