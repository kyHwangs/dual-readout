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
#include "TFile.h"

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include <tuple>
#include <fstream>
#include <sstream>


int main(int argc, char* argv[]) {
  std::string filename = argv[1];
  double fEnergy = std::stod(argv[2]);

  std::vector<std::string> fProcessor;
  std::vector<double> fTime;

  std::ifstream fCSV(Form("%s/cpu_time.csv", filename.c_str()));
  if (fCSV.is_open()) {
    std::string line;

    while (std::getline(fCSV, line)) {
      
      std::stringstream ss(line);
      std::string idx_str, name, time_str;
      std::getline(ss, idx_str, ',');
      std::getline(ss, name, ',');
      std::getline(ss, time_str, ',');
      
      int idx = std::stoi(idx_str);
      double time = std::stod(time_str);
      
      name.erase(0, name.find_first_not_of(" \t\n\r\f\v"));
      name.erase(name.find_last_not_of(" \t\n\r\f\v") + 1);

      fProcessor.push_back(name);
      fTime.push_back(time);
    }
    fCSV.close();
  } 

  gStyle->SetOptFit(1);

  struct HistoSet {

    HistoSet(std::string fName_, double fEnergy_)
    : fName(fName_) {

      fEdep = new TH1D(Form("%s_Edep", fName.c_str()), "", 200, 0, 200);
      fEdep->Sumw2();

      fSTime = new TH1D(Form("%s_Stime", fName.c_str()), "", 600, 10., 70.);
      fSTime->Sumw2();

      double fMaxHit = 400000. * fEnergy_;
      fSHit = new TH1D(Form("%s_Shit", fName.c_str()), "", 400, 0., fMaxHit);
      fSHit->Sumw2();

      fSWave = new TH1D(Form("%s_SWave", fName.c_str()), "", 120, 300., 900.);
      fSWave->Sumw2();
    }

    void Write(TFile* fFile) {
      fFile->cd();
      this->fEdep->Write();
      this->fSTime->Write();
      this->fSHit->Write();
      this->fSWave->Write();
    }

    std::string fName;

    TH1D* fEdep;

    TH1D* fSTime;
    TH1D* fSHit;
    TH1D* fSWave;
  };

  std::map<std::string, HistoSet> fHistoSet;
  fHistoSet.emplace("ALL", HistoSet("ALL", fEnergy));

  for (std::size_t i = 0; i < fProcessor.size(); i++) {

    std::cout << i << " " << Form("%s/root/output_%d.root", filename.c_str(), i) << std::endl;

    if (fHistoSet.find(fProcessor[i]) == fHistoSet.end())
      fHistoSet.emplace(fProcessor[i], HistoSet(fProcessor[i], fEnergy));

    RootInterface<DRsimInterface::DRsimEventData>* drInterface 
      = new RootInterface<DRsimInterface::DRsimEventData>(Form("%s/root/output_%d.root", filename.c_str(), i), false);
    drInterface->GetChain("DRsim");

    unsigned int entries = drInterface->entries();
    while (drInterface->numEvt() < entries) {

      DRsimInterface::DRsimEventData drEvt;
      drInterface->read(drEvt);

      float fEdep = 0.;
      for (const auto fItrEdep : drEvt.Edeps)
        fEdep += fItrEdep.Edep;

      fHistoSet.at("ALL").fEdep->Fill(fEdep / 1000.);
      fHistoSet.at(fProcessor[i]).fEdep->Fill(fEdep / 1000.);

      int fSHits = 0;
      for (const auto fItrTower : drEvt.towers) {        
        for (const auto fItrSiPM : fItrTower.SiPMs) {

          for(const auto fItrTime : fItrSiPM.timeStruct) {
            fHistoSet.at("ALL").fSTime->Fill((fItrTime.first.first + fItrTime.first.second) / 2., fItrTime.second);
            fHistoSet.at(fProcessor[i]).fSTime->Fill((fItrTime.first.first + fItrTime.first.second) / 2., fItrTime.second);
            fSHits += fItrTime.second;
          }

          for(const auto fItrWave : fItrSiPM.wavlenSpectrum) {
            fHistoSet.at("ALL").fSWave->Fill(fItrWave.first.first, fItrWave.second);
            fHistoSet.at(fProcessor[i]).fSWave->Fill(fItrWave.first.first, fItrWave.second);
          }
        }
      }

      fHistoSet.at("ALL").fSHit->Fill(fSHits);
      fHistoSet.at(fProcessor[i]).fSHit->Fill(fSHits);
    }
  }

  TFile* fFile = new TFile(Form("%s/summary.root", filename.c_str()), "RECREATE");

  for (auto& [key, value] : fHistoSet)
    value.Write(fFile);

  fFile->Close();
}
