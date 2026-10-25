#include "bs.hpp"

#include <pybind11/pybind11.h>

#include <sstream>
#include <string>

namespace py = pybind11;

namespace {

std::string repr(const BSParams& p) {
    std::ostringstream os;
    os << "BSParams(S=" << p.S << ", K=" << p.K << ", r=" << p.r << ", sigma=" << p.sigma
       << ", T=" << p.T << ")";
    return os.str();
}

std::string repr(const Quote& q) {
    std::ostringstream os;
    os << "Quote(price=" << q.price << ", delta=" << q.delta << ", gamma=" << q.gamma
       << ", vega=" << q.vega << ", theta=" << q.theta << ")";
    return os.str();
}

} // namespace

// std::invalid_argument thrown by the C++ code is raised as ValueError in Python.
PYBIND11_MODULE(bs_engine, m) {
    m.doc() = "Python bindings for the Black-Scholes pricing engine";

    py::class_<BSParams>(m, "BSParams")
        .def(py::init<>())
        .def(py::init([](double S, double K, double r, double sigma, double T) {
                 return BSParams{S, K, r, sigma, T};
             }),
             py::arg("S"), py::arg("K"), py::arg("r"), py::arg("sigma"), py::arg("T"))
        .def_readwrite("S", &BSParams::S, "spot price of the underlying")
        .def_readwrite("K", &BSParams::K, "strike price")
        .def_readwrite("r", &BSParams::r, "risk-free rate, per year")
        .def_readwrite("sigma", &BSParams::sigma, "volatility, per year")
        .def_readwrite("T", &BSParams::T, "time to maturity, in years")
        .def("__repr__", [](const BSParams& p) { return repr(p); });

    py::class_<Quote>(m, "Quote")
        .def(py::init<>())
        .def_readwrite("price", &Quote::price)
        .def_readwrite("delta", &Quote::delta)
        .def_readwrite("gamma", &Quote::gamma)
        .def_readwrite("vega", &Quote::vega, "per 1.00 change in sigma")
        .def_readwrite("theta", &Quote::theta, "per year")
        .def("__repr__", [](const Quote& q) { return repr(q); });

    py::class_<BlackScholes>(m, "BlackScholes")
        .def(py::init<const BSParams&>(), py::arg("params"))
        .def_property_readonly("params", &BlackScholes::params)
        .def("call_price", &BlackScholes::call_price)
        .def("put_price", &BlackScholes::put_price)
        .def("delta_call", &BlackScholes::delta_call)
        .def("delta_put", &BlackScholes::delta_put)
        .def("gamma", &BlackScholes::gamma)
        .def("vega", &BlackScholes::vega)
        .def("theta_call", &BlackScholes::theta_call)
        .def("theta_put", &BlackScholes::theta_put);

    m.def(
        "quote",
        [](const std::string& style, const BSParams& params) {
            return make_option(style, BlackScholes(params))->quote();
        },
        py::arg("style"), py::arg("params"), "Price and Greeks of a 'call' or 'put' option");

    m.def(
        "call_quote",
        [](const BSParams& params) { return CallOption(BlackScholes(params)).quote(); },
        py::arg("params"), "Price and Greeks of a call option");

    m.def(
        "put_quote",
        [](const BSParams& params) { return PutOption(BlackScholes(params)).quote(); },
        py::arg("params"), "Price and Greeks of a put option");
}
