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

/******************************************************************************
 * This file contains an adaptation of code from the GNU Scientific Library (GSL)
 *
 * specfunc/dilog.c
 *
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2004 Gerard Jungman
 *
 ******************************************************************************/

#pragma once
#include <cmath>
#include <limits>

namespace som {

/* Evaluate series for real dilog(x)
 * \sum_{k=1}{\infty} x^k / k^2
 *
 * Converges rapidly for |x| < 1/2.
 */
double dilog_series_1(double x) {
 const int kmax = 1000;
 double sum  = x;
 double term = x;
 for(int k = 2; k<kmax; ++k) {
  const double rk = (k-1.0)/k;
  term *= x;
  term *= rk*rk;
  sum += term;
  if(std::abs(term/sum) < std::numeric_limits<double>::epsilon()) break;
 }
 return sum;
}

/* Compute the associated series
 *
 *   \sum_{k=1}{\infty} r^k / (k^2 (k+1))
 *
 * This is a series which appears in the one-step accelerated
 * method, which splits out one elementary function from the
 * full definition of Li_2(x). See below.
 */
double series_2(double r) {
 static const int kmax = 100;
 double rk = r;
 double sum = 0.5 * r;
 int k;
 double ds;
 for(k=2; k<10; ++k) {
  rk *= r;
  ds = rk/(k*k*(k+1.0));
  sum += ds;
 }
 for(; k<kmax; ++k) {
  rk *= r;
  ds = rk/(k*k*(k+1.0));
  sum += ds;
  if(std::abs(ds/sum) < 0.5*std::numeric_limits<double>::epsilon()) break;
 }
 return sum;
}

/* Compute Li_2(x) using the accelerated series representation.
 *
 * Li_2(x) = 1 + (1-x)ln(1-x)/x + series_2(x)
 *
 * assumes: -1 < x < 1
 */
double dilog_series_2(double x) {
 double res = series_2(x);
 double t;
 if(x > 0.01) {
  using std::log;
  t = (1.0 - x) * log(1.0-x) / x;
 } else {
  static const double c3 = 1.0/3.0;
  static const double c4 = 1.0/4.0;
  static const double c5 = 1.0/5.0;
  static const double c6 = 1.0/6.0;
  static const double c7 = 1.0/7.0;
  static const double c8 = 1.0/8.0;
  const double t68 = c6 + x*(c7 + x*c8);
  const double t38 = c3 + x *(c4 + x *(c5 + x * t68));
  t = (x - 1.0) * (1.0 + x*(0.5 + x*t38));
 }
 res += 1.0 + t;
 return res;
}

/* Calculates Li_2(x) for real x. Assumes x >= 0.0. */
std::complex<double> dilog_xge0(double x) {
 using std::log;
 if(x > 2.0) {
  const double ser = dilog_series_2(1.0/x);
  const double log_x = log(x);
  const double t1 = M_PI*M_PI/3.0;
  const double t2 = ser;
  const double t3 = 0.5*log_x*log_x;
  return t1 - t2 - t3 - 1_j*M_PI*log(x);
 } else if(x > 1.01) {
  const double ser = dilog_series_2(1.0 - 1.0/x);
  const double log_x = log(x);
  const double log_term = log_x * (log(1.0-1.0/x) + 0.5*log_x);
  const double t1 = M_PI*M_PI/6.0;
  const double t2 = ser;
  const double t3 = log_term;
  return t1 + t2 - t3 - 1_j*M_PI*log(x);
 } else if(x > 1.0) {
  /* series around x = 1.0 */
  const double eps = x - 1.0;
  const double lne = log(eps);
  const double c0 = M_PI*M_PI/6.0;
  const double c1 =   1.0 - lne;
  const double c2 = -(1.0 - 2.0*lne)/4.0;
  const double c3 =  (1.0 - 3.0*lne)/9.0;
  const double c4 = -(1.0 - 4.0*lne)/16.0;
  const double c5 =  (1.0 - 5.0*lne)/25.0;
  const double c6 = -(1.0 - 6.0*lne)/36.0;
  const double c7 =  (1.0 - 7.0*lne)/49.0;
  const double c8 = -(1.0 - 8.0*lne)/64.0;
  return c0+eps*(c1+eps*(c2+eps*(c3+eps*(c4+eps*(c5+eps*(c6+eps*(c7+eps*c8)))))))
         - 1_j*M_PI*log(x);
 } else if(x == 1.0) {
  return M_PI*M_PI/6.0;
 } else if(x > 0.5) {
  const double ser = dilog_series_2(1.0-x);
  const double log_x = log(x);
  const double t1 = M_PI*M_PI/6.0;
  const double t2 = ser;
  const double t3 = log_x*log(1.0-x);
  return t1 - t2 - t3;
 } else if(x > 0.25) {
  return dilog_series_2(x);
 } else if(x > 0.0) {
  return dilog_series_1(x);
 } else {
  /* x == 0.0 */
  return 0;
 }
}

std::complex<double> dilog(double x) {
 return x >= 0 ? dilog_xge0(x) : -dilog_xge0(-x) + 0.5*dilog_xge0(x*x);
}

}
