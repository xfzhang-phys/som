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

#include <array>
#include <utility>

#include <triqs/mc_tools.hpp>

#include "cache_index.hpp"
#include "config_update.hpp"
#include "kernels/all.hpp"
#include "numerics/expabs_distribution.hpp"

namespace som {

template <typename> struct mc_data;

// Common part of all elementary updates
template <typename KernelType> class elementary_update {

protected:
  mc_data<KernelType>& data;
  triqs::mc_tools::random_generator& rng;
  cache_index& ci;
  KernelType const& kern;

#ifdef EXT_DEBUG
  std::pair<double, double> energy_window;
  double width_min;
  double weight_min;
#endif

  // Generate change of a parameter :math:`\delta\xi` (see Sec. 3.4)
  expabs_distribution<triqs::mc_tools::random_generator>
      generate_parameter_change;

  enum { full = 0, half = 1, opt = 2 } selected_parameter_change;

  // Configuration updates generated by :math:`\delta\xi`, :math:`\delta\xi/2`
  // and :math:`\delta\xi_{opt}:
  std::array<config_update, 3> update;

  // :math:`D(\xi+\delta\xi)`, :math:`D(\xi+\delta\xi/2)` and
  // :math:`D(\xi+\delta\xi_{opt})`
  std::array<double, 3> new_objf_value;

  // Returns (true,:math:`\delta\xi_{opt}`) (see eq. (41)), if
  // :math:`\delta\xi_{opt} corresponds to a minimum within [dxi_min, dxi_min[
  // Otherwise returns (false,0)
  std::pair<bool, double> optimize_parameter_change(double dxi, double dxi_min,
                                                    double dxi_max);

  // Make the final choice between :math:`\delta\xi`, :math:`\delta\xi/2`,
  // and, optionally, :math:`\delta\xi_{opt}`.
  void select_parameter_change(bool consider_opt);

  // Returns Metropolis ratio for the currently selected update
  [[nodiscard]] double transition_probability() const;

public:
#ifdef EXT_DEBUG
  elementary_update(mc_data<KernelType>& data,
                    triqs::mc_tools::random_generator& rng, cache_index& ci,
                    std::pair<double, double> energy_window, double width_min,
                    double weight_min);
#else
  elementary_update(mc_data<KernelType>& data,
                    triqs::mc_tools::random_generator& rng, cache_index& ci);
#endif

  double accept();
  void reject();
};

EXTERN_TEMPLATE_CLASS_FOR_EACH_KERNEL(elementary_update)

} // namespace som
