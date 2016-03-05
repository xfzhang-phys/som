/*******************************************************************************
 *
 * TRIQS: a Toolbox for Research in Interacting Quantum Systems
 *
 * Copyright (C) 2016 by I. Krivenko
 *
 * TRIQS is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * TRIQS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * TRIQS. If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#pragma once

#include <array>
#include <cmath>
#include <algorithm>

#include <triqs/mc_tools.hpp>
#include <triqs/utility/numeric_ops.hpp>

#include "config_update.hpp"

namespace som {

using namespace triqs::mc_tools;

template<typename> struct mc_data;

// Common part of all elementary updates
template<typename KernelType> class elementary_update {

protected:

 mc_data<KernelType> & data;
 random_generator & rng;

 // Generate change of a parameter :math:`\delta\xi` (see Sec. 3.4)
 //
 // This function returns a real random number dxi from [dxi_min; dxi_max[
 // distributed as P(dxi) = N exp(-gamma*|dxi|/max(|dxi_min|,|dxi_max|))
 double generate_parameter_change(double dxi_min, double dxi_max) const {
  double dxi_min_abs = std::abs(dxi_min);
  double dxi_max_abs = std::abs(dxi_max);
  double L = std::max(dxi_min_abs, dxi_max_abs);
  double gL = data.gamma / L;

  double x = rng();

  double f = (1-x) * std::copysign(1 - std::exp(-gL * dxi_min_abs), dxi_min) +
                 x * std::copysign(1 - std::exp(-gL * dxi_max_abs), dxi_max);

  return std::copysign(-std::log(1 - std::abs(f)) / gL, f);
 }

 enum {full = 0, half = 1, opt = 2} selected_parameter_change;

 // Configuration updates generated by :math:`\delta\xi`, :math:`\delta\xi/2` and :math:`\delta\xi_{opt}:
 std::array<config_update,3> update;

 // :math:`D(\xi+\delta\xi)`, :math:`D(\xi+\delta\xi/2)` and :math:`D(\xi+\delta\xi_{opt})`
 std::array<double,3> new_objf_value;

 // Returns (true,:math:`\delta\xi_{opt}`) (see eq. (41)), if :math:`\delta\xi_{opt}
 // corresponds to a minimum within [dxi_min, dxi_min[
 // Otherwise returns (false,0)
 std::pair<bool,double> optimize_parameter_change(double dxi, double dxi_min, double dxi_max) {
  new_objf_value[full] = data.objf(update[full]);
  new_objf_value[half] = data.objf(update[half]);

  double a = 2 * (data.temp_objf_value - 2 * new_objf_value[half] + new_objf_value[full]) / (dxi*dxi);
  if(a <= 0) return std::make_pair(false,0); // dxi_opt is going to be a maximum
  double b = (4 * new_objf_value[half] - 3 * data.temp_objf_value - new_objf_value[full]) / dxi;

  double dxi_opt = -b / (2*a);

  // dxi_opt is outside the definition domain
  if(dxi_opt < dxi_min || dxi_opt >= dxi_max) return std::make_pair(false,0);

  else return std::make_pair(true,dxi_opt);
 }

 // Make the final choice between :math:`\delta\xi`, :math:`\delta\xi/2`,
 // and, optionally, :math:`\delta\xi_{opt}`.
 void select_parameter_change(bool consider_opt) {
  if(consider_opt) {
   new_objf_value[opt] = data.objf(update[opt]);
   selected_parameter_change = opt;
   if(new_objf_value[full] < new_objf_value[opt]) selected_parameter_change = full;
   if(new_objf_value[half] < new_objf_value[full]) selected_parameter_change = half;
  } else {
   selected_parameter_change = (new_objf_value[full] < new_objf_value[half]) ? full : half;
  }
 }

 // Returns Metropolis ratio for the currently selected update
 double transition_probability() const {
  double old_d = data.temp_objf_value;
  double new_d = new_objf_value[selected_parameter_change];
  return new_d < old_d ? 1.0 : data.Z(old_d / new_d);
 }

public:

 elementary_update(mc_data<KernelType> & data, random_generator & rng) :
  data(data), rng(rng),
  update{config_update(data.temp_conf), config_update(data.temp_conf), config_update(data.temp_conf)},
  new_objf_value{0,0,0}
  {}

 //----------------

 double accept() {
  // Update temporary configuration ...
  update[selected_parameter_change].apply();
  // and its objective function value
  data.temp_objf_value = new_objf_value[selected_parameter_change];

  // Reset all considered updates
  update[full].reset();
  update[half].reset();
  update[opt].reset();

#ifdef EXT_DEBUG
    std::cerr << "* Elementary update accepted" << std::endl;
    std::cerr << "Temporary configuration: size = " << data.temp_conf.size()
              << ", norm = " << data.temp_conf.norm()
              << ", D = " << data.temp_objf_value << std::endl;
    using triqs::utility::is_zero;
    if(!is_zero(data.temp_conf.norm() - data.global_conf.norm(), 1e-10))
     TRIQS_RUNTIME_ERROR << "Elementary update has changed norm of the solution!";
#endif

  // Copy this temporary configuration to the globally selected configuration
  // if its objective function value is smaller
  if(data.temp_objf_value < data.global_objf_value) {
#ifdef EXT_DEBUG
    std::cerr << "Copying temporary configuration to global configuration "
              << "(D(temp) = " << data.temp_objf_value
              << ", D(global) = " << data.global_objf_value << ")" << std::endl;
#endif
   data.global_conf = data.temp_conf;
   data.global_objf_value = data.temp_objf_value;
  }

#ifdef EXT_DEBUG
    std::cerr << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
#endif

  // Update internal state of the distribution function
  ++data.Z;

  return 1;
 }

 //----------------

 void reject() {

#ifdef EXT_DEBUG
    std::cerr << "* Elementary update rejected" << std::endl;
    std::cerr << "Temporary configuration: size = " << data.temp_conf.size()
              << ", norm = " << data.temp_conf.norm()
              << ", D = " << data.temp_objf_value << std::endl;
    std::cerr << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
#endif

  // Reset all considered updates
  update[full].reset();
  update[half].reset();
  update[opt].reset();

  // Update internal state of the distribution function
  ++data.Z;
 }

};

}