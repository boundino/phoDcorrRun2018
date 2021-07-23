#ifndef __MCANA_H__
#define __MCANA_H__

namespace mcana_
{
  std::vector<std::string> types = {
    "mc_isoR4", "mc_isoR3",
    "gen_isoR4", "gen_isoR3",
    "gen_Kpi_isoR4", "gen_Kpi_isoR3",
  };
  std::map<std::string, TH1F*> hdphi;

  std::map<std::string, std::vector<float>> djt_bins_pp = {
    {"dphi", {0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0}},
    {"dr", {0, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 1.0}},
  };
  std::map<std::string, std::vector<float>> djt_bins_aa = {
    {"dphi", {0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0}},
    // {"dphi", {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0}},
    {"dr", {0, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 1.0}},
  };
}

#endif
