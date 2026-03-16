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
    .def("theta_put", &BlackScholes::theta_put);
}