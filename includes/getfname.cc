#include "param.h"
#include <iostream>

int main(int argc, char* argv[])
{
  if(argc==7)
    { 
      phoD::param pa(atoi(argv[1]), atof(argv[2]), atof(argv[3]), atof(argv[4]), atof(argv[5]), atof(argv[6]), "Q");
      std::cout<<pa.tag()<<std::endl;
      return 0; 
    }
  return 1;
}
