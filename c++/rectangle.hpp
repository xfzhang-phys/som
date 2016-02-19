#pragma once

#include <vector>
#include <complex>
#include <ostream>

namespace som {

using namespace triqs::arrays;

struct rectangle {

 double center;             // center
 double width;              // width
 double height;             // height

 rectangle() = default;
 rectangle(double center, double width, double height) :
  center(center), width(width), height(height) {}
 rectangle(rectangle const&) = default;
 rectangle(rectangle &&) = default;
 rectangle & operator=(rectangle const&) = default;
 rectangle & operator=(rectangle &&) = default;

 double norm() const { return width * height; }
 double operator()(double x) const {
  return (x >= center - width/2 && x <= center + width/2) ? height : 0;
 }

 std::complex<double> hilbert_transform(std::complex<double> z) const {
  return -height*std::log((center + width/2 - z)/(center - width/2 - z));
 }
 vector<double> tail_coefficients(long order_min, long order_max) const {
  vector<double> data(order_max - order_min + 1);

  double e1 = center - width/2, e2 = center + width/2;
  double e1n = 1, e2n = 1;
  for(long n = order_min; n <= order_max; ++n) {
   if(n < 1) { data(n-order_min) = 0; continue; }
   e1n *= e1; e2n *= e2;
   data(n-order_min) = height*(e2n - e1n)/n;
  }

  return data;
 }

 // stream insertion
 friend std::ostream& operator<<(std::ostream& os, rectangle r) {
  os << "(c:" << r.center << ", w:" << r.width << ", h:" << r.height << ")";
  return os;
 }

 // multiplication by scalar
 friend rectangle operator*(rectangle const& r, double alpha) {
  return {r.center, r.width, r.height * alpha};
 }
 friend rectangle operator*(double alpha, rectangle const& r) { return r*alpha; }

};

}
