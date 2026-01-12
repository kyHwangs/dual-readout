#include "RootInterface.h"
#include "RecoInterface.h"
#include "DRsimInterface.h"
#include "functions.h"

#include "TROOT.h"
#include "TStyle.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TPaveStats.h"
#include "TString.h"
#include "TLorentzVector.h"
#include "TGraph.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
  TString filename = argv[1];

  gStyle->SetOptFit(1);


  TH1F* tE_C = new TH1F("E_C","Energy of Cerenkov ch.;GeV;Evt",100,0,3000);
  tE_C->Sumw2();

  TH1F* tE_S = new TH1F("E_S","Energy of Scintillation ch.;GeV;Evt",100,0,30000);
  tE_S->Sumw2();
  
  TH1F* tP_leak = new TH1F("Pleak","Momentum leak;MeV;Evt",100,0.,20000);
  tP_leak->Sumw2();
  
  TH1F* tP_leak_nu = new TH1F("Pleak_nu","Neutrino energy leak;MeV;Evt",100,0.,20000);
  tP_leak_nu->Sumw2();

  TH1F* tEdep = new TH1F("totEdep","Total Energy deposit;MeV;Evt",100,0.,21000);
  tEdep->Sumw2();

  TH1F* tT_C = new TH1F("time_C","Cerenkov time;ns;p.e.",150,10.,70.);
  tT_C->Sumw2();

  TH1F* tT_S = new TH1F("time_S","Scint time;ns;p.e.",150,10.,70.);
  tT_S->Sumw2();

  TH1F* tWav_S = new TH1F("wavlen_S","Scint wavelength;nm;p.e.",120,300.,900.);
  tWav_S->Sumw2();

  TH1F* tWav_C = new TH1F("wavlen_C","Cerenkov wavelength;nm;p.e.",120,300.,900.);
  tWav_C->Sumw2();

  TH1F* tNhit_S = new TH1F("nHits_S","Number of Scint p.e./SiPM;p.e.;n",200,0.,200.);
  tNhit_S->Sumw2();
  
  TH1F* tNhit_C = new TH1F("nHits_C","Number of Cerenkov p.e./SiPM;p.e.;n",50,0.,50.);
  tNhit_C->Sumw2();
  
  RootInterface<DRsimInterface::DRsimEventData>* drInterface = new RootInterface<DRsimInterface::DRsimEventData>(std::string(filename));
  drInterface->set("DRsim","DRsimEventData");


  unsigned int entries = drInterface->entries();
  while (drInterface->numEvt() < entries) {
    if (drInterface->numEvt() % 100 == 0) printf("Analyzing %dth event ...\n", drInterface->numEvt());

    DRsimInterface::DRsimEventData drEvt;
    drInterface->read(drEvt);

    float Edep = 0.;
    for (auto edepItr = drEvt.Edeps.begin(); edepItr != drEvt.Edeps.end(); ++edepItr) {
      auto edep = *edepItr;
      Edep += edep.Edep;
    }
    tEdep->Fill(Edep);

    float Pleak = 0.;
    float Eleak_nu = 0.;
    for (auto leak : drEvt.leaks) {
      TLorentzVector leak4vec;
      leak4vec.SetPxPyPzE(leak.px,leak.py,leak.pz,leak.E);
      if ( std::abs(leak.pdgId)==12 || std::abs(leak.pdgId)==14 || std::abs(leak.pdgId)==16 ) {
        Eleak_nu += leak4vec.P();
      } else {
        Pleak += leak4vec.P();
      }
    }
    tP_leak->Fill(Pleak);
    tP_leak_nu->Fill(Eleak_nu);

    double nHitC = 0;
    double nHitS = 0;

    for (auto tower = drEvt.towers.begin(); tower != drEvt.towers.end(); ++tower) {
      for (auto sipm = tower->SiPMs.begin(); sipm != tower->SiPMs.end(); ++sipm) {
        if ( RecoInterface::IsCerenkov(sipm->x,sipm->y) ) {
          tNhit_C->Fill(sipm->count);

          for (const auto timepair : sipm->timeStruct) {
            tT_C->Fill(timepair.first.first+0.05,timepair.second);
            if (timepair.first.first < 36.) nHitC+=timepair.second;
          }
          for (const auto wavpair : sipm->wavlenSpectrum) {
            tWav_C->Fill(wavpair.first.first,wavpair.second);
          }
        } else {
          tNhit_S->Fill(sipm->count);

          for (const auto timepair : sipm->timeStruct) {
            tT_S->Fill(timepair.first.first+0.05,timepair.second);
            if (timepair.first.first < 60.) nHitS+=timepair.second;
          }
          for (const auto wavpair : sipm->wavlenSpectrum) {
            tWav_S->Fill(wavpair.first.first,wavpair.second);
          }
        }
      }
    }

    tE_C->Fill(nHitC);
    tE_S->Fill(nHitS);
  } // event loop

  TCanvas* c = new TCanvas("c","");

  tEdep->Draw("Hist"); c->SaveAs("plot_Edep.png");

  c->SetLogy(1);
  tP_leak->Draw("Hist"); c->SaveAs("plot_Pleak.png");
  tP_leak_nu->Draw("Hist"); c->SaveAs("plot_Pleak_nu.png");
  c->SetLogy(0);

  tE_C->Draw("Hist"); c->SaveAs("plot_EC.png");
  tE_S->Draw("Hist"); c->SaveAs("plot_ES.png");
  tT_C->Draw("Hist"); c->SaveAs("plot_tC.png");
  tT_S->Draw("Hist"); c->SaveAs("plot_tS.png");
  tWav_C->Draw("Hist"); c->SaveAs("plot_wavC.png");
  tWav_S->Draw("Hist"); c->SaveAs("plot_wavS.png");
  tNhit_C->Draw("Hist"); c->SaveAs("plot_nhitC.png");
  tNhit_S->Draw("Hist"); c->SaveAs("plot_nhitS.png");

  TFile* tRoot = new TFile("output.root","RECREATE");
  tRoot->cd();
  tE_C->Write();
  tE_S->Write();
  tT_C->Write();
  tT_S->Write();
  tWav_C->Write();
  tWav_S->Write();
  tNhit_C->Write();
  tNhit_S->Write();
  tRoot->Close();
}
