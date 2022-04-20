namespace mcana_
{
  void seth(TH1F* h, bool forcemaxdigits=false);
  void makecanvas(xjjroot::mypdf* fpdf, Djet::param& pa, TLegend* leg);
}

void mcana_::seth(TH1F* h, bool forcemaxdigits)
{
  h->SetMinimum(0);
  h->SetMaximum(h->GetMaximum()*1.6);
  h->GetXaxis()->SetNdivisions(505);
  if(forcemaxdigits) h->GetYaxis()->SetMaxDigits(1);
  xjjroot::sethempty(h, 0, 0.1);
}


void mcana_::makecanvas(xjjroot::mypdf* fpdf, Djet::param& pa, TLegend* leg)
{
  pa.drawtex(0.23, 0.85, 0.035, "cent");
  leg->Draw();
  xjjroot::drawCMSleft("Simulation");
  xjjroot::drawCMSright("#sqrt{s_{NN}} = 5.02 TeV");
  // xjjroot::drawtex(0.92, 0.80, (xjjc::str_contains(fpdf->getfilename(), "embed")?"PYTHIA + HEDJET":"PYTHIA 8"), 0.035, 33, 62);
  auto note = xjjc::str_eraseall(fpdf->getfilename(), "plots/");
  note = xjjc::str_eraseall(note, "cgen.pdf");
  xjjroot::drawcomment(note, "lb");
  fpdf->write();
}

