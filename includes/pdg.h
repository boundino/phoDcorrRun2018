#ifndef __PDG_H__
#define __PDG_H__

namespace pdg
{
  const float DZERO_MASS = 1.8649;
  const float BR_DZERO_KPI = 0.0395;
}

namespace pdg
{
  const float DZERO_SIGREG = 0.045; // half
  const float DZERO_BKG_L = 0.08; // half
  const float DZERO_BKG_H = 0.13; // half
  float DZERO_SCALE = DZERO_SIGREG/(DZERO_BKG_H-DZERO_BKG_L);
  bool dzero_sigreg(float Dmass) { return (fabs(Dmass-DZERO_MASS)<DZERO_SIGREG); }
  bool dzero_sideband(float Dmass) { return (fabs(Dmass-DZERO_MASS)>DZERO_BKG_L && fabs(Dmass-DZERO_MASS)<DZERO_BKG_H); }
}

#endif
