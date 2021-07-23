#include <string>
#include "mvaprod.h"

void mvaprobsimple_main(std::string inputname, std::string treename, std::string outputname, std::string weightdir,
                        std::string rootfname="")
{
  int nevt = -1;
  mytmva::mvaprob(inputname, treename, outputname, weightdir, nevt, rootfname);
}

int main(int argc, char* argv[])  
{
  if(argc==6)
    {
      mvaprobsimple_main(argv[1], argv[2], argv[3], argv[4], argv[5]);
      return 0;
    }
  if(argc==5)
    {
      mvaprobsimple_main(argv[1], argv[2], argv[3], argv[4]);
      return 0;
    }
  return 1;
}
