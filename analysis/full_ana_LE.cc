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
#include "TH2.h"
#include "TProfile.h"

#include <iostream>
#include <string>

float GetDepthScint(float time) {
  // peak: return (21.6544 - time) / 2.9284;
  // LE 30%
  return (19.5016 - time) / 2.55;
}

float GetDepthCeren(float time) {
  // peak return (19.2205 - time) / 1.9549;
  // LE 30%
  return (18.3927 - time) / 1.76;
}

float GetInterpolate(float x1, float y1, float x2, float y2, float yThres) {
  return x2 + (yThres - y2) * (x2 - x1) / (y2 - y1);
}

float GetLeadingEdge(TH1F* tHist, float tThreshold) {
  if (tHist->GetEntries() < 100) return 0.;

  float tPeak = tHist->GetBinCenter(tHist->GetMaximumBin());
  float tThres = tPeak * tThreshold;

  for (int i = 1; i <= tHist->GetNbinsX(); i++) {
    if (tHist->GetBinContent(i) > tThres) {

      return GetInterpolate(tHist->GetBinCenter(i - 1), tHist->GetBinContent(i - 1), tHist->GetBinCenter(i), tHist->GetBinContent(i), tThres);
    
    }
  }

}

int main(int argc, char* argv[]) {
  TString cases = argv[1];
  TString particle = argv[2];

  gStyle->SetOptFit(1);

  TString tFilename = "";
  TString tOutputSuffix = "";
  float tCalibCeren = 0.;
  float tCailbScint = 0.;

  if (cases == "CC7") {
    tFilename = "CC7_pi20.root";
    tOutputSuffix = "CC7_pi_";
    tCalibCeren = 73.4;
    tCailbScint = 1125.55;
    if (particle == "ele") {
      tFilename = "CC7_ele20.root";
      tOutputSuffix = "CC7_ele_";
    }
  } else if (cases == "CC7_NoEdge") {
    tFilename = "CC7_NoEdge_pi20.root";
    tOutputSuffix = "CC7NoEdge_pi_";
    tCalibCeren = 73.4;
    tCailbScint = 1125.55;
    if (particle == "ele") {
      tFilename = "CC7_NoEdge_ele20.root";
      tOutputSuffix = "CC7NoEdge_ele_";
    }
  } else if (cases == "AL9") {
    tFilename = "Alma9_pi20.root";
    tOutputSuffix = "AL9_pi_";
    tCalibCeren = 73.6 * 19.8 / 20.;
    tCailbScint = 1131.15;
    if (particle == "ele") {
      tFilename = "Alma9_ele20.root";
      tOutputSuffix = "AL9_ele_";
    }
  } else if (cases == "AL9_NoEdge") {
    tFilename = "Alma9_NoEdge_pi20.root";
    tOutputSuffix = "AL9NoEdge_pi_";
    tCalibCeren = 72.8 * 19.8 / 20.;
    tCailbScint = 1114.3;
    if (particle == "ele") {
      tFilename = "Alma9_NoEdge_ele20.root";
      tOutputSuffix = "AL9NoEdge_ele_";
    }
  } else if (cases == "AL9_NoEdge_noScint") {
    tFilename = "Alma9_NoEdge_pi20_noScint.root";
    tOutputSuffix = "AL9NoEdgeNoScint_pi_";
    tCalibCeren = 72.8;
    tCailbScint = 1114.3;
    if (particle == "ele") {
      tFilename = "Alma9_NoEdge_ele20_noScint.root";
      tOutputSuffix = "AL9NoEdgeNoScint_ele_";
    }
  }


  std::cout << "Processing - " << cases << " " << particle << " " << tFilename << std::endl;

  TH1F* tE_C = new TH1F("E_C",";Energy [GeV];Evt",120,0,30);
  tE_C->Sumw2();

  TH1F* tE_S = new TH1F("E_S",";Energy [GeV];Evt",120,0,30);
  tE_S->Sumw2();
  
  TH1F* tP_leak = new TH1F("Pleak",";Momentum [MeV];Evt",100,0.,20000);
  tP_leak->Sumw2();
  
  TH1F* tP_leak_nu = new TH1F("Pleak_nu",";Neutrino energy [MeV];Evt",100,0.,20000);
  tP_leak_nu->Sumw2();

  TH1F* tEdep = new TH1F("totEdep",";Energy [MeV];Evt",100,0.,21000);
  tEdep->Sumw2();

  TH1F* tT_C = new TH1F("time_C",";Time [ns];p.e.",600,10.,70.);
  tT_C->Sumw2();

  TH1F* tT_S = new TH1F("time_S",";Time [ns];p.e.",600,10.,70.);
  tT_S->Sumw2();

  TH1F* tTmax_C = new TH1F("tTmax_C",";Time [ns];p.e.",150,10.,25.);
  tTmax_C->Sumw2();

  TH1F* tTmax_S = new TH1F("tTmax_S",";Time [ns];p.e.",150,10.,25.);
  tTmax_S->Sumw2();


  TH1F* tWav_S = new TH1F("wavlen_S",";Wavelength [nm];p.e.",120,300.,900.);
  tWav_S->Sumw2();

  TH1F* tWav_C = new TH1F("wavlen_C",";Wavelength [nm];p.e.",120,300.,900.);
  tWav_C->Sumw2();

  TH1F* tNhit_S = new TH1F("nHits_S",";Number of Scint p.e./SiPM;# SiPMs",200,0.,200.);
  tNhit_S->Sumw2();
  
  TH1F* tNhit_C = new TH1F("nHits_C",";Number of Cerenkov p.e./SiPM;# SiPMs",50,0.,50.);
  tNhit_C->Sumw2();

  TH1F* tDepth_C = new TH1F("tDepth_C",";Depth [m];Evt", 30, -0.5, 2.5);
  tDepth_C->Sumw2();

  TH1F* tDepth_S = new TH1F("tDepth_S",";Depth [m];Evt", 30, -0.5, 2.5);
  tDepth_S->Sumw2();

  TH2F* tDepth_vs_E_C = new TH2F("tDepth_vs_E_C",";Depth [m];Energy [MeV]", 30, -0.5, 2.5, 120,0,30);
  tDepth_vs_E_C->Sumw2();
  tDepth_vs_E_C->SetStats(0);

  TH2F* tDepth_vs_E_S = new TH2F("tDepth_vs_E_S",";Depth [m];Energy [MeV]", 30, -0.5, 2.5, 120,0,30);
  tDepth_vs_E_S->Sumw2();
  tDepth_vs_E_S->SetStats(0);
  
  TCanvas* c = new TCanvas("c","");

  RootInterface<DRsimInterface::DRsimEventData>* drInterface = new RootInterface<DRsimInterface::DRsimEventData>(std::string(tFilename));
  drInterface->set("DRsim","DRsimEventData");

  unsigned int entries = drInterface->entries();
  while (drInterface->numEvt() < entries) {
    // if (drInterface->numEvt() % 100 == 0) printf("Analyzing %dth event ...\n", drInterface->numEvt());
    if (drInterface->numEvt() == entries - 1) std::cout << "Last events" << std::endl;

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

    float nHitC = 0.;
    float nHitS = 0.;

    TH1F* tST_C = new TH1F("singleT_C","",600,10.,70.);
    TH1F* tST_S = new TH1F("singleT_S","",600,10.,70.);  

    for (auto tower = drEvt.towers.begin(); tower != drEvt.towers.end(); ++tower) {
      for (auto sipm = tower->SiPMs.begin(); sipm != tower->SiPMs.end(); ++sipm) {
        if ( RecoInterface::IsCerenkov(sipm->x,sipm->y) ) {
          tNhit_C->Fill(sipm->count);

          for (const auto timepair : sipm->timeStruct) {
            tT_C->Fill(timepair.first.first+0.05,timepair.second);
            if (timepair.first.first < 31.55) {
              nHitC+=timepair.second;
              tST_C->Fill(timepair.first.first+0.05,timepair.second);
            }
          }
          for (const auto wavpair : sipm->wavlenSpectrum) {
            tWav_C->Fill(wavpair.first.first,wavpair.second);
          }
        } else {
          tNhit_S->Fill(sipm->count);

          for (const auto timepair : sipm->timeStruct) {
            tT_S->Fill(timepair.first.first+0.05,timepair.second);
            if (timepair.first.first < 60.){
              nHitS+=timepair.second;
              tST_S->Fill(timepair.first.first+0.05,timepair.second);
            }
          }
          for (const auto wavpair : sipm->wavlenSpectrum) {
            tWav_S->Fill(wavpair.first.first,wavpair.second);
          }
        }
      }
    }

    if (drInterface->numEvt() < 5) {
      c->cd();

      tST_C->Draw();
      c->SaveAs(tOutputSuffix + "TimeC_SingleEvent" + std::to_string(drInterface->numEvt()) + ".png");

      tST_S->Draw();
      c->SaveAs(tOutputSuffix + "TimeS_SingleEvent" + std::to_string(drInterface->numEvt()) + ".png");
    }

    float tE_Ceren = nHitC / tCalibCeren;
    float tE_Scint = nHitS / tCailbScint;

    float tMaxTimeCeren = tST_C->GetBinCenter(tST_C->GetMaximumBin());
    float tMaxTimeScint = tST_S->GetBinCenter(tST_S->GetMaximumBin());
    
    tTmax_C->Fill(GetLeadingEdge(tST_C, 0.3));
    tTmax_S->Fill(GetLeadingEdge(tST_S, 0.3));

    float tMaxDepthCeren = GetDepthCeren(GetLeadingEdge(tST_C, 0.3));
    float tMaxDepthScint = GetDepthScint(GetLeadingEdge(tST_S, 0.3));

    delete tST_C;
    delete tST_S;

    tE_C->Fill(tE_Ceren);
    tE_S->Fill(tE_Scint);
    tDepth_C->Fill(tMaxDepthCeren);
    tDepth_S->Fill(tMaxDepthScint);
    tDepth_vs_E_C->Fill(tMaxDepthCeren, tE_Ceren);
    tDepth_vs_E_S->Fill(tMaxDepthScint, tE_Scint);
  } // event loop

    
  auto tDepthProfileX_C = tDepth_vs_E_C->ProfileX();
  tDepthProfileX_C->GetXaxis()->SetTitle("Depth [m]");
  tDepthProfileX_C->GetYaxis()->SetTitle("Energy [GeV]");
  tDepthProfileX_C->SetMarkerStyle(20);
  tDepthProfileX_C->SetMarkerSize(0.5);
  tDepthProfileX_C->SetMarkerColor(kBlack);
  tDepthProfileX_C->SetLineColor(kBlack);

  auto tDepthProfileX_S = tDepth_vs_E_S->ProfileX();
  tDepthProfileX_S->GetXaxis()->SetTitle("Depth [m]");
  tDepthProfileX_S->GetYaxis()->SetTitle("Energy [GeV]");
  tDepthProfileX_S->SetMarkerStyle(20);
  tDepthProfileX_S->SetMarkerSize(0.5);
  tDepthProfileX_S->SetMarkerColor(kBlack);
  tDepthProfileX_S->SetLineColor(kBlack);


  TString attFitFunc = "[1] * exp([0] * x)";
  
  TF1* tAttFit_C = new TF1("tAttFit_C", attFitFunc, 0., 1.1);
  tAttFit_C->SetLineColor(kRed);
  tDepthProfileX_C->SetOption("p"); 
  tDepthProfileX_C->Fit(tAttFit_C,"R+&same");
  tDepthProfileX_C->GetYaxis()->SetRangeUser(0., 50.);
  tDepthProfileX_C->Draw("pe");
  c->SaveAs(tOutputSuffix + "AttFit_C.png");

  TF1* tAttFit_S = new TF1("tAttFit_S", attFitFunc, 0., 1.2);
  tAttFit_S->SetLineColor(kRed);
  tDepthProfileX_S->SetOption("p"); 
  tDepthProfileX_S->Fit(tAttFit_S,"R+&same");
  tDepthProfileX_S->GetYaxis()->SetRangeUser(0., 50.);
  tDepthProfileX_S->Draw("pe");
  c->SaveAs(tOutputSuffix + "AttFit_S.png");

  tEdep->Draw("Hist"); c->SaveAs(tOutputSuffix + "Edep.png");

  c->SetLogy(1);
  tP_leak->Draw("Hist"); c->SaveAs(tOutputSuffix + "Pleak.png");
  tP_leak_nu->Draw("Hist"); c->SaveAs(tOutputSuffix + "Pleak_nu.png");
  c->SetLogy(0);

  tE_C->Draw("Hist"); c->SaveAs(tOutputSuffix + "EC.png");
  tE_S->Draw("Hist"); c->SaveAs(tOutputSuffix + "ES.png");
  tT_C->Draw("Hist"); c->SaveAs(tOutputSuffix + "tC.png");
  tT_S->Draw("Hist"); c->SaveAs(tOutputSuffix + "tS.png");
  tWav_C->Draw("Hist"); c->SaveAs(tOutputSuffix + "wavC.png");
  tWav_S->Draw("Hist"); c->SaveAs(tOutputSuffix + "wavS.png");
  tNhit_C->Draw("Hist"); c->SaveAs(tOutputSuffix + "nhitC.png");
  tNhit_S->Draw("Hist"); c->SaveAs(tOutputSuffix + "nhitS.png");
  tDepth_C->Draw("Hist"); c->SaveAs(tOutputSuffix + "depthC.png");
  tDepth_S->Draw("Hist"); c->SaveAs(tOutputSuffix + "depthS.png");
  tDepth_vs_E_C->Draw("COLZ"); c->SaveAs(tOutputSuffix + "depth_vs_E_C.png");
  tDepth_vs_E_S->Draw("COLZ"); c->SaveAs(tOutputSuffix + "depth_vs_E_S.png");
  tTmax_C->Draw("Hist"); c->SaveAs(tOutputSuffix + "tTmaxC.png");
  tTmax_S->Draw("Hist"); c->SaveAs(tOutputSuffix + "tTmaxS.png");

  TFile* tFile = new TFile(tOutputSuffix + "Analysis.root","RECREATE");
  tDepthProfileX_C->Write();
  tDepthProfileX_S->Write();
  tAttFit_C->Write();
  tAttFit_S->Write();
  tFile->Close();


  // std::cout << "Ceren: " << tTmax_C->GetBinCenter(tTmax_C->GetMaximumBin()) << " " << tT_C->GetBinCenter(tT_C->GetMaximumBin()) << std::endl;
  // std::cout << "Scint: " << tTmax_S->GetBinCenter(tTmax_S->GetMaximumBin()) << " " << tT_S->GetBinCenter(tT_S->GetMaximumBin()) << std::endl;


  return 0;
  //            Ceren Scint
  // CC7        18.85.21.35           
  // AL9        18.85.21.35                 
  // AL9_NoEdge 18.85 21.35

  // Ceren: 18.95 18.95
  // Scint: 21.25 21.25
}
