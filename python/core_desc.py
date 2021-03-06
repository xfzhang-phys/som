# Generated automatically using the command :
# c++2py.py ../c++/som_core.hpp --compiler_options=-DCACHE_SIZE=0x10000 -p -mpytriqs.applications.analytical_continuation.som.core -o core --moduledoc "The Stochastic Optimization Method"
from wrap_generator import *

# The module
module = module_(full_name = "pytriqs.applications.analytical_continuation.som.core",
                 doc = "The Stochastic Optimization Method")

# All the triqs C++/Python modules
module.use_module('gf', 'triqs')
module.use_module('histograms', 'triqs')

# Add here all includes beyond what is automatically included by the triqs modules
module.add_include("som_core.hpp")

# Add here anything to add in the C++ code at the start, e.g. namespace using
module.add_preamble("""
#include <triqs/python_tools/converters/pair.hpp>
#include <triqs/python_tools/converters/tuple.hpp>
#include <triqs/python_tools/converters/vector.hpp>
using namespace triqs::gfs;
using namespace triqs::statistics;
using namespace som;
#include "./converters.hxx"
""")

module.add_enum("observable_kind", ["FermionGf","BosonCorr","BosonAutoCorr","ZeroTemp"], "som", "Kinds of observables")

# The class som_core
c = class_(
        py_type = "SomCore",        # name of the python class
        c_type = "som_core",        # name of the C++ class
        doc = r"Main class of SOM", # doc of the C++ class
)

c.add_constructor("""(gf_view<imtime> g_tau, gf_view<imtime> S_tau, som::observable_kind kind = FermionGf, vector<double> norms = {})""",
                  doc = """Construct on imaginary-time quantities """)

c.add_constructor("""(gf_view<imfreq> g_iw, gf_view<imfreq> S_iw, som::observable_kind kind = FermionGf, vector<double> norms = {})""",
                  doc = """Construct on imaginary-frequency quantities """)

c.add_constructor("""(gf_view<legendre> g_l, gf_view<legendre> S_l, som::observable_kind kind = FermionGf, vector<double> norms = {})""",
                  doc = """Construct on quantities in Legendre polynomial basis """)

c.add_method("""void run (**som::run_parameters_t)""",
             release_GIL_and_enable_signal = True,
             doc = open('parameters.rst', 'r').read())

lshift = pyfunction(name = '__lshift__', arity = 2)
lshift.add_overload(calling_pattern = '() =', signature = '(gf_view<refreq> g_w, som::som_core s)')
lshift.add_overload(calling_pattern = '() =', signature = '(gf_view<imtime> g_tau, som::som_core s)')
lshift.add_overload(calling_pattern = '() =', signature = '(gf_view<imfreq> g_iw, som::som_core s)')
lshift.add_overload(calling_pattern = '() =', signature = '(gf_view<legendre> g_l, som::som_core s)')
lshift.treat_as_inplace = True
c.number_protocol['lshift'] = lshift

c.add_property(name = "last_run_parameters",
               getter = cfunction("som::run_parameters_t get_last_run_parameters ()"),
               doc = """Set of parameters used in the last call to run() """)

c.add_property(name = "run_status",
               getter = cfunction("int get_run_status ()"),
               doc = """Status of the run on exit """)

solutions_cp = """
auto const& solutions = self_c.get_solutions();
std::vector<std::vector<std::tuple<double,double,double>>> result;
for(auto const& sol : solutions) result.emplace_back(sol.begin(), sol.end());
"""
c.add_property(name = "solutions",
               getter = cfunction(signature = "std::vector<std::vector<std::tuple<double,double,double>>>()",
                                  calling_pattern = solutions_cp),
               doc = """Accumulated solutions as lists of rectangle parameter tuples (center,width,height)""")

c.add_property(name = "histograms",
               getter = cfunction("std::vector<histogram> get_histograms ()"),
               doc = """Accumulated objective function histograms""")

module.add_class(c)

module.generate_code()
