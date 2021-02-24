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
}

#endif
