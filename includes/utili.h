#ifndef __PHOD_UTILI__
#define __PHOD_UTILI__

#include <cmath>

namespace phoD
{
  float cal_dphi_0pi(float phi1, float phi2);
  float cal_dphi_01(float phi1, float phi2);
}

float phoD::cal_dphi_0pi(float phi1, float phi2)
{
  // phi1, phi2: -pi ~ pi
  float dphi = phi1 - phi2; // -2pi ~ 2pi
  dphi = fabs(dphi); // 0 ~ 2pi
  if(dphi > M_PI) dphi = 2*M_PI - dphi; // 0 ~ pi

  return dphi;
}

float phoD::cal_dphi_01(float phi1, float phi2)
{
  float dphi = cal_dphi_0pi(phi1, phi2);
  dphi /= M_PI;

  return dphi;
}

#endif
