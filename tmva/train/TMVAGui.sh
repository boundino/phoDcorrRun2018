#!/bin/bash

echo '
#include <string>
#include "TMVA/TMVAGui.h"

int tmvagui(std::string inputname)
{
  TMVA::TMVAGui( inputname.c_str() );
  return 0;
}
' > tmvagui.C

##
[[ -f $1 ]] && root "tmvagui.C+(\"$1\")" || echo -e "$0: error: invalid input file."

rm tmvagui.C