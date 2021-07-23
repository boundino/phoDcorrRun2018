#include <iostream>
#include <string>
#include <vector>

#include <TFile.h>
#include "xjjcuti.h"

#include "forest.h"

int printxs(std::string inputname)
{
  TFile* inf = TFile::Open(inputname.c_str()); // lowest pthat sample
  phoD::forest* f = new phoD::forest(inf, true);
  phoD::etree* etr = f->etr();

  std::vector<float> pthats = {30, 50, 80, 120};
  std::vector<int> nevt(pthats.size(), 0);

  int nentries = f->GetEntries();
  for(int i=0; i<nentries; i++)
    {
      xjjc::progressslide(i, nentries, 100000);
      etr->GetEntry(i);

      for(int p=0; p<pthats.size(); p++)
        {
          if(etr->pthat() >= pthats[p]) nevt[p]++;
        }
    }
  xjjc::progressbar_summary(nentries);
  std::cout<<std::endl;
  for(int p=0; p<pthats.size(); p++)
    {
      std::cout<<std::left<<"pthat > "<<std::setw(5)<<Form("%.0f:", pthats[p])<<nevt[p]<<std::endl<<std::string(20, '-')<<std::endl;
    }
  std::cout<<std::endl;
}

int main(int argc, char* argv[])
{
  if(argc==2) return printxs(argv[1]);
  return 1;
}
