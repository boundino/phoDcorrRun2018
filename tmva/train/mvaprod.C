#include <string>

#include "TMVAClassification.h"
#include "mvaprod.h"

void mvaprob_main(std::string inputname, std::string treename, bool ishi, std::string outputname, std::string outputfiledir,
                  std::string mymethod = "", std::string stage = "0,1,2,3,4,5,6,7,8,9,10")
{
  std::string outputfilename = xjjc::str_replaceall(xjjc::str_divide(inputname, "/").back(), ".root", "");
  outputfilename = outputfiledir + "/" + outputfilename;
  std::vector<std::string> weightdirs(mytmva::nptbins);
  for(int i=0; i<mytmva::nptbins; i++)
    {
      std::string outfname = xjjc::str_replaceallspecial(mytmva::mkname(outputname, mytmva::ptbins[i], mytmva::ptbins[i+1], mymethod, stage));
      std::string weightdir = Form("dataset/weights/%s", outfname.c_str());
      weightdirs[i] = weightdir;
    }
  mytmva::mvaprob(inputname, "Bfinder/ntmix", ishi, outputfilename, weightdirs);
}

int main(int argc, char* argv[])  
{
  if(argc==8)
    {
      mvaprob_main(argv[1], argv[2], atoi(argv[3]), argv[4], argv[5], argv[6], argv[7]);
      return 0;
    }
  return 1;
}
