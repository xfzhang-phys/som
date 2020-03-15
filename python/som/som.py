##############################################################################
#
# SOM: Stochastic Optimization Method for Analytic Continuation
#
# Copyright (C) 2016-2020 Igor Krivenko <igor.s.krivenko@gmail.com>
#
# SOM is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# SOM is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# SOM. If not, see <http://www.gnu.org/licenses/>.
#
##############################################################################
"""
Main module of SOM
"""

from som_core import SomCore
import numpy as np

class Som(SomCore):
    """Stochastic Optimization Method"""

    def __init__(self, g, s = None, kind = "FermionGf", norms = np.array([])):
        if s is None:
            s = g.copy()
            s.data[:,Ellipsis] = np.eye(s.target_shape[0])
        if isinstance(norms,float) or isinstance(norms,int):
            norms = norms * np.ones((g.target_shape[0],))
        SomCore.__init__(self, g, s, kind, norms)

def count_good_solutions(hist, upper_lim = 1):
    """
    Given a histogram of objective function values,
    count the number of solutions with D/D_{min} <= 1 + upper_lim
    """
    d_max = hist.limits[0] * (1 + upper_lim)
    return int(sum(c for n, c in enumerate(hist.data) if hist.mesh_point(n) <= d_max))
