#ifndef MODEL_BASE_MODEL_H
#define MODEL_BASE_MODEL_H

#include "basedef.h"
#include "data/data_point.h"
#include <boost/shared_ptr.hpp>
#include <boost/math/tools/roots.hpp>
#include <boost/bind/bind.hpp>
#include <boost/ref.hpp>
#include <iostream>

typedef boost::function<mat(const mat&, const data_point&)> grad_func_type;

class base_model;
//TODO
template<typename MODEL>
class Get_grad_coeff;
template<typename MODEL>
class Implicit_fn;

class base_model {
  /**
   * Base class for models
   *
   * @param experiment list of attributes to take from R type
   */
public:
  // Constructors
  base_model(Rcpp::List experiment) {
    name = Rcpp::as<std::string>(experiment["name"]);
    lambda1 = Rcpp::as<double>(experiment["lambda1"]);
    lambda2 = Rcpp::as<double>(experiment["lambda2"]);
  }

  // Gradient
  grad_func_type grad_func();
  mat gradient(const mat& theta_old, const data_point& data_pt) const;

  // Members
  std::string name;
  double lambda1;
  double lambda2;
};

template<typename MODEL>
class Get_grad_coeff {
  // Compute gradient coeff and its derivative for Implicit-SGD update
public:
  Get_grad_coeff(const MODEL& e, const data_point& d,
    const mat& t, double n) :
    model(e), data_pt(d), theta_old(t), normx(n) {}

  double operator() (double ksi) const {
    return data_pt.y-model.h_transfer(dot(theta_old, data_pt.x)
                     + normx * ksi);
  }

  double first_derivative (double ksi) const {
    return model.h_first_derivative(dot(theta_old, data_pt.x)
           + normx * ksi)*normx;
  }

  double second_derivative (double ksi) const {
    return model.h_second_derivative(dot(theta_old, data_pt.x)
             + normx * ksi)*normx*normx;
  }

  const MODEL& model;
  const data_point& data_pt;
  const mat& theta_old;
  double normx;
};

template<typename MODEL>
class Implicit_fn {
  // Root finding functor for Implicit-SGD update
public:
  typedef boost::math::tuple<double, double, double> tuple_type;

  Implicit_fn(double a, const Get_grad_coeff<MODEL>& get_grad) :
    at(a), g(get_grad) {}

  tuple_type operator() (double u) const {
    double value = u - at * g(u);
    double first = 1 + at * g.first_derivative(u);
    double second = at * g.second_derivative(u);
    tuple_type result(value, first, second);
    return result;
  }

  double at;
  const Get_grad_coeff<MODEL>& g;
};

#endif