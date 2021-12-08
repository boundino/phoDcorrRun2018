
namespace emix
{
  const int N_HIBIN = 90;
  const float min_HIBIN = 0, max_HIBIN = 180;
  const int N_VZ = 30;
  const float min_VZ = -15, max_VZ = 15;
  const int N_EVENTPLANE = 16;
  const float min_EVENTPLANE = -M_PI/2., max_EVENTPLANE = M_PI/2.;

  class mixhve
  {
  public:
    mixhve(int nHIBIN_, int nVZ_, int nEVENTPLANE_) : nHIBIN(nHIBIN_), nVZ(nVZ_), nEVENTPLANE(nEVENTPLANE_) { ; }
    int getBin(int hiBin, float vz, double eventPlaneAngle);
    int ihibin(int hiBin);
    int ivz(float vz);
    int ieventplane(float eventPlaneAngle);

  private:
    int nHIBIN;
    int nVZ;
    int nEVENTPLANE;
  };
}

int emix::mixhve::getBin(int hiBin, float vz, double eventPlaneAngle)
{
  int ihibin_ = ihibin(hiBin);
  int ivz_ = ivz(vz);
  int ieventplane_ = ieventplane(eventPlaneAngle);
  if(ihibin_ < 0 || ivz_ < 0 || ieventplane_ < 0) return -1;
  int idx = nHIBIN*(nVZ*ieventplane_ + ivz_) + ihibin_;
  return idx;
}

int emix::mixhve::ivz(float vz)
{
  for (int i = 0; i < nVZ; ++i){
    if ((i+min_VZ) <= vz && vz < (i+min_VZ+1)) return i;
  }
  if (vz == 15) return nVZ-1;
  return -1;
}

int emix::mixhve::ihibin(int hiBin)
{
  int div = (max_HIBIN-min_HIBIN) / nHIBIN;
  if(hiBin < min_HIBIN || hiBin > max_HIBIN) return -1;
  return hiBin/div;
}

int emix::mixhve::ieventplane(float eventPlaneAngle)
{
  if(nEVENTPLANE==1) return 0;
  for (int i = 0; i < nEVENTPLANE; ++i){
    if (eventPlaneAngle - min_EVENTPLANE >= i*(max_EVENTPLANE - min_EVENTPLANE)/nEVENTPLANE && 
        eventPlaneAngle - min_EVENTPLANE < (i+1)*(max_EVENTPLANE - min_EVENTPLANE)/nEVENTPLANE) return i;
  }
  if (eventPlaneAngle - min_EVENTPLANE == max_EVENTPLANE - min_EVENTPLANE) return nEVENTPLANE-1;
  return -1;
}

