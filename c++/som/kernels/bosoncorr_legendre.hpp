/*******************************************************************************
 *
 * SOM: Stochastic Optimization Method for Analytic Continuation
 *
 * Copyright (C) 2016-2020 Igor Krivenko <igor.s.krivenko@gmail.com>
 *
 * SOM is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * SOM is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * SOM. If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#pragma once

#include <vector>
#include <cmath>
#include <boost/math/special_functions/bessel.hpp>
#include <triqs/gfs.hpp>
#include <triqs/utility/numeric_ops.hpp>

#include "base.hpp"
#include "../numerics/spline.hpp"
#include "../numerics/polynomial.hpp"
#include "../numerics/simpson.hpp"

namespace som {

using namespace triqs::arrays;
using namespace triqs::gfs;

// Kernel: fermionic GF, Legendre basis
template<> class kernel<BosonCorr,legendre> :
           public kernel_base<kernel<BosonCorr,legendre>, array<double,1>> {

 // Tolerance levels for function evaluation
 static constexpr double tolerance = 1e-14;
 // Number of energy points for spline interpolation
 static constexpr int n_spline_knots = 15001;
 // Starting low-energy/high-energy boundary value for l=0
 static constexpr double x0_start_l0 = 15.0;
 // Step used in x0 boundary search
 static constexpr double x0_step = 1.0;

 // Make coefficient of a Bessel polynomial a_k(l+1/2)
 static double make_a(int k, int l) {
  double a = 1;
  for(int i = 1; i <= k; ++i) {
   double t = l-k+2*i;
   a *= (t-1)*t/double(2*i);
  }
  return a;
 }

 // Evaluator object
 // operator(z) returns (2/(\pi*\beta)) * sqrt(2*l+1) * \int_0^z i_l(x) / cosh(x) dx
 struct evaluator {
  double x0;                        // Boundary between low-energy and high-energy regions
  regular_spline low_energy_spline; // Spline approximation on [0;x0]
  polynomial<> high_energy_pol;     // Polynomial approximation on ]x0;\infty[
  double log_coeff;                 // -a_1(l+1/2) = -l(l+1)/2
  double low_energy_x0;             // low_energy_spline(x0)
  double high_energy_x0;            // x - log_coeff * log(x0) + high_energy_pol(1/x0)
  double pref;                      // (2/(\pi*\beta)) * sqrt(2*l+1) prefactor

  evaluator(int l, double x0_start, double beta) :
   log_coeff(-0.5*l*(l+1)), pref((2/(M_PI*beta)) * std::sqrt(2*l+1)) {

   // Integrand, x i_l(x) / sinh(x)
   auto integrand = [l](double x) {
    if(x==0) return (l==0 ? 1.0 : 0.0);
    double val = boost::math::cyl_bessel_i(l+0.5, x);
    return val * std::sqrt(M_PI/(2*x)) * x / std::sinh(x);
   };

   vector<double> tail_coeffs(l+1);
   for(int k = 0; k <= l; ++k)
    tail_coeffs[k] = (k%2 ? -1 : 1) * make_a(k,l);
   polynomial<> integrand_tail(tail_coeffs);

   // Search for the low-energy/high-energy boundary
   x0 = x0_start;
   using triqs::utility::is_zero;
   while(!is_zero(integrand(x0)-integrand_tail(1/x0), tolerance)) {
    x0 += x0_step;
    // http://en.cppreference.com/w/cpp/numeric/math/sinh
    if(x0 >= 710)
     TRIQS_RUNTIME_ERROR << "kernel<BosonCorr,legendre>: l = " << l
                         << " is too large and may cause numerical overflows.";
   }

   // Fill high_energy_pol
   vector<double> int_tail_coeffs(l);
   if(l>0) {
    int_tail_coeffs[0] = 0;
    for(int k = 1; k <= l-1; ++k) int_tail_coeffs[k] = -tail_coeffs[k+1]/k;
   }
   high_energy_pol = polynomial<>(int_tail_coeffs);

   // Fill low_energy_spline
   auto spline_knots = primitive(integrand, .0, x0, n_spline_knots, tolerance);
   low_energy_spline = regular_spline(0, x0, spline_knots);

   // Set low_energy_x0 and high_energy_x0
   low_energy_x0 = low_energy_spline(x0);
   high_energy_x0 = x0 + log_coeff * std::log(x0) + high_energy_pol(1/x0);
  }

  double operator()(double x) const {
   double val = (x <= x0) ?
                low_energy_spline(x) :
                (low_energy_x0 - high_energy_x0 +
                 (x + log_coeff * std::log(x) + high_energy_pol(1/x)));
   return pref * val;
  }
 };

 // Evaluator objects, one object per l
 std::vector<evaluator> evaluators;

 // Integrated kernel \Lambda(l,\Omega)
 double Lambda(int l, double Omega) const {
  return -(l%2 ? 1.0 : std::copysign(1.0, -Omega)) *
          evaluators[l](std::abs(Omega)*beta/2);
 }

public:

 using result_type = array<double,1>;
 using mesh_type = gf_mesh<legendre>;
 constexpr static observable_kind kind = BosonCorr;

 const double beta;    // Inverse temperature
 const mesh_type mesh; // Legendre coefficients mesh

 kernel(mesh_type const& mesh) :
  kernel_base(mesh.size()), beta(mesh.domain().beta), mesh(mesh) {
  evaluators.reserve(mesh.size());

  double x0 = x0_start_l0;
  for(auto l : mesh) {
   evaluators.emplace_back(l, x0, beta);
   x0 = evaluators.back().x0;
  }
 }

 // Apply to a rectangle
 void apply(rectangle const& rect, result_type & res) const {

  double e1 = rect.center - rect.width/2;
  double e2 = rect.center + rect.width/2;

  for(auto l : mesh) res(l.linear_index()) = rect.height * (Lambda(l, e2) - Lambda(l, e1));
 }

 friend std::ostream & operator<<(std::ostream & os, kernel const& kern) {
  os << "A(\\epsilon) -> \\chi(l), ";
  os << "\\beta = " << kern.beta << ", " << kern.mesh.size() << " Legendre coefficients.";
  return os;
 }

};

}
