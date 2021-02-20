
namespace emix
{
  const int N_HIBIN = 100;
  const int N_VZ = 30;
  // const int N_EVENTPLANE = 16;
  const int N_EVENTPLANE = 1;

  class mixhve
  {
  public:
    mixhve(int nHIBIN_, int nVZ_, int nEVENTPLANE_) : nHIBIN(nHIBIN_), nVZ(nVZ_), nEVENTPLANE(nEVENTPLANE_) { ; }
    int getBin(int hiBin, float vz, double eventPlaneAngle);
    int ihibin(int hiBin);
    int ivz(float vz);
    int ieventplane(double eventPlaneAngle);

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
    if ((i-15) <= vz && vz < (i-14)) return i;
  }
  if (vz == 15) return 29;
  return -1;
}

int emix::mixhve::ihibin(int hiBin)
{
  int div = 200 / nHIBIN;
  if(hiBin < 0 || hiBin > 200) return -1;
  return hiBin/div;
}

int emix::mixhve::ieventplane(double eventPlaneAngle)
{
  if(nEVENTPLANE==1) return 0;
  for (int i = 0; i < nEVENTPLANE; ++i){
    if ((double)i*M_PI/nEVENTPLANE <= eventPlaneAngle + 0.5*M_PI && eventPlaneAngle + 0.5*M_PI < (double)(i+1)*M_PI/nEVENTPLANE) return i;
  }
  if (eventPlaneAngle + 0.5*M_PI == (double)M_PI) return 15;
  return -1;
}
