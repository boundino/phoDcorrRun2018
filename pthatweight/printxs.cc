#include <iostream>
#include <string>
#include <vector>

#include <TFile.h>
#include "xjjcuti.h"

#include "forest.h"

int printxs(std::string inputname)
{
  TFile* inf = TFile::Open(inputname.c_str()); // lowest pthat sample
  phoD::forest* f = new phoD::forest(inf);
  phoD::etree* etr = f->etr();

  std::vector<float> pthats = {15, 30, 50, 80, 120};
  std::vector<int> nevt(pthats.size(), 0);

  int nentries = f->GetEntries();
  for(int i=0; i<nentries; i++)
    {
      if(i%10000==0 || i==nentries-1) xjjc::progressbar(i, nentries);
      f->GetEntry(i);

      for(int p=0; p<pthats.size(); p++)
        {
          if(etr->pthat() >= pthats[p]) nevt[p]++;
        }
    }
  xjjc::progressbar_summary(nentries);
  for(int p=0; p<pthats.size(); p++)
    {
      std::cout<<pthats[p]<<" "<<nevt[p]<<std::endl;
    }
}

int main(int argc, char* argv[])
{
  if(argc==2) return printxs(argv[1]);
  return 1;
}
