#include "bs.hpp"
#include <cmath>

double BlackScholes::norm_cdf(double x) const {
    return 0.5 * (1 + std::erf(x / std::sqrt(2.0)));
}

double BlackScholes::norm_pdf(double x) const {
    const double inv_sqrt_2pi = 1.0 / std::sqrt(2.0 * M_PI);
    return inv_sqrt_2pi * std::exp(-0.5 * x * x);
}

double BlackScholes::d1() const{
    return (std::log(p_.S/p_.K) + (p_.r + 0.5 * (p_.sigma*p_.sigma))*p_.T)/(p_.sigma*std::sqrt(p_.T));
} 

double BlackScholes::d2() const{
    return d1() - p_.sigma * std::sqrt(p_.T);
}

double BlackScholes::call_price() const{
    return (p_.S * norm_cdf(d1())) - p_.K * std::exp(-p_.r*p_.T)*norm_cdf(d2());
}

double BlackScholes::put_price() const{    
    return (call_price() - p_.S + (p_.K * std::exp(-p_.r * p_.T)));
}

double BlackScholes::delta_call() const {
    return norm_cdf(d1());
}

double BlackScholes::delta_put() const {
    return norm_cdf(d1()) - 1;
}

double BlackScholes::gamma() const {
    return (norm_pdf(d1()))/(p_.S*p_.sigma*std::sqrt(p_.T));
}

double BlackScholes::vega() const{
    return p_.S*norm_pdf(d1())*std::sqrt(p_.T);
}

CallOption::CallOption(const BlackScholes& model) : model_(model) {
    
}

Quote CallOption::quote() const
{
    Quote q;

    q.price = model_.call_price();
    q.delta = model_.delta_call();
    q.gamma = model_.gamma();
    q.vega  = model_.vega();

    return q;
}

PutOption::PutOption(const BlackScholes& model) : model_(model) {
    
}

Quote PutOption::quote() const
{
    Quote q;

    q.price = model_.put_price();
    q.delta = model_.delta_put();
    q.gamma = model_.gamma();
    q.vega  = model_.vega();

    return q;
}