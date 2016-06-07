/*******************************************************************************
 *
 * SOM: Stochastic Optimization Method for Analytic Continuation
 *
 * Copyright (C) 2016 by I. Krivenko
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

#include <triqs/arrays/array.hpp>
#include <triqs/utility/numeric_ops.hpp>

namespace som {

// Simpson's rule
template<typename X, typename F>
auto simpson(F f, X a, X b) -> decltype(f(a)*a) {
 return (b-a)/6 * (f(a) + 4*f((a+b)/2) + f(b));
}

template<typename X, typename F>
auto adaptive_simpson_impl(F f, X a, X b, X eps, X whole) -> decltype(f(a)*a) {
 auto c = (a+b)/2;
 auto left = simpson(f, a, c);
 auto right = simpson(f, c, b);
 auto d = left + right - whole;

 using triqs::utility::is_zero;
 if(is_zero(d, 15*eps)) return left + right + d/15;
 return adaptive_simpson_impl(f, a, c, eps/2, left)
      + adaptive_simpson_impl(f, c, b, eps/2, right);
}

// Adaptive Simpson's method
template<typename X, typename F>
auto adaptive_simpson(F f, X x_min, X x_max, X eps) -> decltype(f(x_min)*x_min) {
 return adaptive_simpson_impl(f, x_min, x_max, eps, simpson(f, x_min, x_max));
}

// Compute F(X_i) = \int_{x_min}^X f(x) dx, X_i = x_min, ..., x_max
template<typename X, typename F>
auto primitive(F f, X x_min, X x_max, int N, X eps) -> triqs::arrays::array<decltype(f(x_min)*x_min),1> {
 using res_type = decltype(f(x_min)*x_min);
 triqs::arrays::array<res_type,1> res(N);
 res(0) = res_type{};
 auto h = (x_max - x_min)/(N-1);
 for(int i = 0; i < N-1; ++i) {
  auto a = i*h, b = (i+1)*h;
  res(i+1) = res(i) + adaptive_simpson(f, a, b, eps);
 }
 return res;
}

}
