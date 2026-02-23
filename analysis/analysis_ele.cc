#include "RootInterface.h"
#include "RecoInterface.h"
#include "DRsimInterface.h"
#include "functions.h"

#include "TROOT.h"
#include "TStyle.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TPaveStats.h"
#include "TString.h"
#include "TLorentzVector.h"
#include "TGraph.h"
#include "Riostream.h"
#include "TProfile.h"
#include "TGraphErrors.h"

#include <utility>
#include <vector>
#include <iostream>

float GetDepthScint(float time) {

  return 1000. * (21.7016 - time) / 2.55;
}


int main(int argc, char* argv[]) {
  int cases = std::stoi(argv[1]);
  float eBinWidth = 350.;

  gStyle->SetOptFit(1);

  int binNumber = 1000;
  float low = 0;
  float high = 350;

  TString filename = "";
  TString plotname = "";
  TString rootname = (TString)("./plots/ele/" + std::to_string(cases) + ".root");

  float plot_low = 0;
  float plot_high = 0;

  if (cases == 5) { // pi
    filename = "./ele_combined/ele_5GeV.root";
    plotname = "./plots/ele/5/plots";
    plot_low = 0;
    plot_high = 10;
  }

  if (cases == 10) { // pi
    filename = "./ele_combined/ele_10GeV.root";
    plotname = "./plots/ele/10/plots";
    plot_low = 0;
    plot_high = 20;
  }

  if (cases == 20) { // pi
    filename = "./ele_combined/ele_20GeV.root";
    plotname = "./plots/ele/20/plots";
    plot_low = 5;
    plot_high = 35; 
  }

  if (cases == 30) { // pi
    filename = "./ele_combined/ele_30GeV.root";
    plotname = "./plots/ele/30/plots";
    plot_low = 10;
    plot_high = 50;
  }

  if (cases == 50) { // pi
    filename = "./ele_combined/ele_50GeV.root";
    plotname = "./plots/ele/50/plots";
    plot_low = 15;
    plot_high = 85;  
  }

  if (cases == 70) { // pi
    filename = "./ele_combined/ele_70GeV.root";
    plotname = "./plots/ele/70/plots";
    plot_low = 25;
    plot_high = 115;  
  }

  if (cases == 90) { // pi
    filename = "./ele_combined/ele_90GeV.root";
    plotname = "./plots/ele/90/plots";
    plot_low = 40;
    plot_high = 140;  
  }

  if (cases == 110) { // pi
    filename = "./ele_combined/ele_110GeV.root";
    plotname = "./plots/ele/110/plots";
    plot_low = 60;
    plot_high = 160;  
  }

  float tCalibCeren = 72.8 * 395.208 / 400.;
  float tCailbScint = 1114.3 * 400.86 / 400.;

  int Cthres = 31.55;

  TH1F* tE_C = new TH1F("E_C","Energy of Cerenkov ch.;GeV;Evt",binNumber,low,high);
  tE_C->Sumw2(); tE_C->SetLineColor(kBlue); tE_C->SetLineWidth(2);
  TH1F* tE_S = new TH1F("E_S","Energy of Scintillation ch.;GeV;Evt",binNumber,low,high);
  tE_S->Sumw2(); tE_S->SetLineColor(kRed); tE_S->SetLineWidth(2);
  TH1F* tE_Sum = new TH1F("E_Sum","Energy of Sum;GeV;Evt",binNumber,low,high);
  tE_Sum->Sumw2(); tE_Sum->SetLineColor(kBlack); tE_Sum->SetLineWidth(2);


  RootInterface<DRsimInterface::DRsimEventData>* drInterface = new RootInterface<DRsimInterface::DRsimEventData>(std::string(filename));
  drInterface->set("DRsim","DRsimEventData");

  std::vector<float> E_Ss, E_Cs, E_Sums;
  unsigned int entries = drInterface->entries();
  while (drInterface->numEvt() < entries) {
    if (drInterface->numEvt() % 100 == 0) printf("Analyzing %dth event ...\n", drInterface->numEvt());

    DRsimInterface::DRsimEventData drEvt;
    drInterface->read(drEvt);

    float cE_tmp = 0; float sE_tmp = 0;
    for (auto tower = drEvt.towers.begin(); tower != drEvt.towers.end(); ++tower) {

      float chit_tmp = 0; float shit_tmp = 0;
      for (auto sipm = tower->SiPMs.begin(); sipm != tower->SiPMs.end(); ++sipm) {
        if ( RecoInterface::IsCerenkov(sipm->x,sipm->y) ) {

          for (const auto timepair : sipm->timeStruct) {
            if ( timepair.first.first < Cthres )
              chit_tmp += timepair.second;
          }

        } else {

          shit_tmp += sipm->count;
        
        }
      }
      cE_tmp += chit_tmp / tCalibCeren;
      sE_tmp += shit_tmp / tCailbScint;
    }


    E_Cs.push_back(cE_tmp);
    E_Ss.push_back(sE_tmp);
    E_Sums.push_back(cE_tmp + sE_tmp);


    tE_C->Fill(cE_tmp);
    tE_S->Fill(sE_tmp);
    tE_Sum->Fill(cE_tmp + sE_tmp);
  } // event loop
  drInterface->close();

  TF1* grE_C = new TF1("Cfit","gaus",plot_low,plot_high); 
  grE_C->SetLineColor(kBlue);
  tE_C->SetOption("p"); 
  tE_C->Fit(grE_C,"R+&same");

  TF1* grE_S = new TF1("Sfit","gaus",plot_low,plot_high); 
  grE_S->SetLineColor(kRed);
  tE_S->SetOption("p"); 
  tE_S->Fit(grE_S,"R+&same");

  TF1* grE_Sum = new TF1("Sumfit","gaus",2*plot_low,2*plot_high); 
  grE_Sum->SetLineColor(kBlack);
  tE_Sum->SetOption("p"); 
  tE_Sum->Fit(grE_Sum,"R+&same");

  TFile* aFile = new TFile(rootname, "RECREATE");
  aFile->cd();

  tE_C->Write();
  tE_S->Write();
  tE_Sum->Write();

  aFile->Close();

  tE_C->GetXaxis()->SetRangeUser(plot_low, plot_high);
  tE_S->GetXaxis()->SetRangeUser(plot_low, plot_high);
  tE_Sum->GetXaxis()->SetRangeUser(2. * plot_low, 2. * plot_high);


  TCanvas* c = new TCanvas("c","");
  c->cd();

  tE_C->Draw();
  c->SaveAs(plotname+"_Ceren.png");

  tE_S->Draw();
  c->SaveAs(plotname+"_Scint.png");

  tE_Sum->Draw();
  c->SaveAs(plotname+"_Sum.png");




}
