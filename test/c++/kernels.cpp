#include <vector>
#include <triqs/test_tools/arrays.hpp>

#include "kernels/fermiongf_imtime.hpp"
#include "kernels/fermiongf_imfreq.hpp"

using namespace som;
using namespace triqs::gfs;
using triqs::arrays::vector;

cache_index ci;
configuration conf({{-2.0,2.6,0.3,ci},
                    {-1.3,2.6,0.4,ci},
                    {-0.5,2.6,0.5,ci},
                    {1.3,2.6,0.6,ci},
                    {2.0,2.6,0.7,ci}},ci);

TEST(Kernels,FermionicGf_imtime) {

 double beta = 1;
 gf_mesh<imtime> mesh(beta,Fermion,11);
 kernel<FermionGf,imtime> kern(mesh);
 ci.invalidate_all();

 std::vector<vector<double>> ref = {
 {-0.11009,-0.12894,-0.15171,-0.17935,-0.21309,-0.25448,-0.30549,-0.36872,-0.44746,-0.54600,-0.66991},
 {-0.24860,-0.27281,-0.30080,-0.33328,-0.37115,-0.41546,-0.46754,-0.52898,-0.60174,-0.68822,-0.79140},
 {-0.50906,-0.51974,-0.53328,-0.54995,-0.57010,-0.59411,-0.62244,-0.65562,-0.69427,-0.73910,-0.79094},
 {-1.18710,-1.03234,-0.90261,-0.79347,-0.70131,-0.62319,-0.55672,-0.49992,-0.45119,-0.40922,-0.37290},
 {-1.56312,-1.27400,-1.04408,-0.86036,-0.71283,-0.59378,-0.49721,-0.41849,-0.35399,-0.30086,-0.25688}};

 for(int i = 0; i < conf.size(); ++i) EXPECT_TRUE(array_are_close(ref[i],kern(conf[i]),1e-5));
}

TEST(Kernels,FermionicGf_imfreq) {

 double beta = 1;
 gf_mesh<imfreq> mesh(beta,Fermion,5);
 kernel<FermionGf,imfreq> kern(mesh);
 ci.invalidate_all();

 std::vector<vector<dcomplex>> ref = {
  {0.104264-0.177225_j,0.0165217-0.0787992_j,0.0061808-0.0487618_j,0.00318834-0.0351386_j,0.00193761-0.0274307_j},
  {0.104345-0.276539_j,0.0146693-0.107669_j,0.00540573-0.0656136_j,0.00277628-0.0470732_j,0.00168408-0.0366793_j},
  {0.0552639-0.384823_j,0.00716173-0.136697_j,0.00261382-0.0824898_j,0.00133869-0.0590156_j,0.000811104-0.0459314_j},
  {-0.156517-0.414809_j,-0.022004-0.161504_j,-0.0081086-0.0984203_j,-0.00416442-0.0706098_j,-0.00252612-0.055019_j},
  {-0.243283-0.413524_j,-0.0385507-0.183865_j,-0.0144219-0.113777_j,-0.00743947-0.0819901_j,-0.00452108-0.0640049_j}};

 for(int i = 0; i < conf.size(); ++i) EXPECT_TRUE(array_are_close(ref[i],kern(conf[i]),1e-6));
}

MAKE_MAIN;
