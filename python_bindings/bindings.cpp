#include <pybind11/pybind11.h>
#include "bs.hpp"
namespace py = pybind11;

PYBIND11_MODULE(bs_engine,m) {
    m.doc() = "Python bindings for Black-scholes engine";

    py::class_<BSParams>(m, "BSParams")
    .def(py::init<>())
    .def_readwrite("S", &BSParams::S)
    .def_readwrite("K", &BSParams::K)
    .def_readwrite("r", &BSParams::r)
    .def_readwrite("sigma", &BSParams::sigma)
    .def_readwrite("T", &BSParams::T);

    py::class_<Quote>(m, "Quote")
    .def(py::init<>())
    .def_readwrite("price", &Quote::price)
    .def_readwrite("delta", &Quote::delta)
    .def_readwrite("gamma", &Quote::gamma)
    .def_readwrite("vega", &Quote::vega)
    .def_readwrite("theta", &Quote::theta);

    py::class_<BlackScholes>(m, "BlackScholes")
    .def(py::init<const BSParams&>())
    .def("call_price", &BlackScholes::call_price)
    .def("put_price", &BlackScholes::put_price)
    .def("gamma", &BlackScholes::gamma)
    .def("vega", &BlackScholes::vega)
    .def("theta_call", &BlackScholes::theta_call)
    .def("theta_put", &BlackScholes::theta_put)
    .def("delta_call", &BlackScholes::delta_call)
    .def("delta_put", &BlackScholes::delta_put);

    m.def("quote", [](const std::string& style, const BSParams& params) {
        BlackScholes model(params);
        auto option = make_option(style, model);
        return option->quote();
    }, py::arg("style"), py::arg("params"));

    m.def("call_quote", [](const BSParams& params){
        BlackScholes model(params);
        CallOption option(model);
        return option.quote();
    }, py::arg("params"));

    m.def("put_quote", [] (const BSParams& params) {
        BlackScholes model(params);
        PutOption option(model);
        return option.quote();
    }, py::arg("params"));
}