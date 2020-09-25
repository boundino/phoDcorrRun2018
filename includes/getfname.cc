#include "param.h"
#include <iostream>

int main(int argc, char* argv[])
{
  if(argc==6)
    { 
      phoD::param pa(atof(argv[1]), atof(argv[2]), atof(argv[3]), atof(argv[4]), atof(argv[5]), "Q");
      std::cout<<pa.tag()<<std::endl;
      return 0; 
    }
  return 1;
}
