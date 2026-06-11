#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TH1.h"
#include "TColor.h"
#include "TLegend.h"
#include "TFile.h"

static std::vector<std::string> fColors = {
  "#000000",
  "#3f90da",		
  "#ffa90e",		
  "#bd1f01",		
  "#94a4a2",		
  "#832db6",		
  "#a96b59",		
  "#e76300",		
  "#b9ac70",		
  "#717581",		
  "#92dadd"
};


bool parseCpuTimeLine(const std::string& line, int& jobIndex, std::string& cpu, double& time) {
  std::stringstream ss(line);
  std::string jobIndexStr;
  std::string timeStr;

  if (!std::getline(ss, jobIndexStr, ',') ||
      !std::getline(ss, cpu, ',') ||
      !std::getline(ss, timeStr)) {
    return false;
  }

  try {
    jobIndex = std::stoi(jobIndexStr);
  } catch (...) {
    return false;
  }

  if (timeStr.empty()) {
    time = std::numeric_limits<double>::quiet_NaN();
  } else {
    time = std::stod(timeStr);
  }

  return true;
}


bool readCpuTimeCsv(const std::string& csvPath,
                    std::vector<int>& jobIndices,
                    std::vector<std::string>& cpus,
                    std::vector<double>& times) {
  std::ifstream input(csvPath);
  if (!input.is_open()) {
    std::cerr << "Failed to open " << csvPath << std::endl;
    return false;
  }

  std::string line;
  while (std::getline(input, line)) {
    if (line.empty()) {
      continue;
    }

    int jobIndex = 0;
    std::string cpu;
    double time = std::numeric_limits<double>::quiet_NaN();

    // This also skips a header line such as: JobIndex,CPU,time
    if (!parseCpuTimeLine(line, jobIndex, cpu, time)) {
      continue;
    }

    jobIndices.push_back(jobIndex);
    cpus.push_back(cpu);
    times.push_back(time);
  }

  return true;
}

void SetMaximum(std::map<std::string, TH1D*>& fMapHist) {
  double max = 0;
  for (auto itr = fMapHist.begin(); itr != fMapHist.end(); ++itr) {
    if (itr->second->GetMaximum() > max) max = itr->second->GetMaximum();
  }
  for (auto itr = fMapHist.begin(); itr != fMapHist.end(); ++itr) {
    itr->second->GetYaxis()->SetRangeUser(0, max * 1.3);
  }
}


int main(int argc, char* argv[]) {
  std::string csvPath = (argc >= 2) ? argv[1] : "cpu_time.csv";

  std::vector<int> jobIndices;
  std::vector<std::string> cpus;
  std::vector<double> times;

  if (!readCpuTimeCsv(csvPath, jobIndices, cpus, times)) {
    return 1;
  }

  std::cout << "Loaded " << jobIndices.size() << " rows from " << csvPath << std::endl;

  std::map<std::string, TH1D*> fMapHist;

  for (std::size_t i = 0; i < jobIndices.size(); ++i) {

    if (fMapHist.find(cpus[i]) == fMapHist.end())
      fMapHist[cpus[i]] = new TH1D(cpus[i].c_str(), cpus[i].c_str(), 400, 0, 40000); 

    fMapHist[cpus[i]]->Fill(times[i]);
  }

  SetMaximum(fMapHist);

  TCanvas* c = new TCanvas("c","");
  TLegend* l = new TLegend(0.5, 0.5, 0.9, 0.9);
  l->SetFillStyle(0);
  l->SetBorderSize(0);

  TH1D* hCount = new TH1D("Count","Count", fMapHist.size(), 0, fMapHist.size());

  hCount->GetXaxis()->SetTitle("CPU");
  hCount->GetYaxis()->SetTitle("Count");

  int i = 0;
  for (auto itr = fMapHist.begin(); itr != fMapHist.end(); ++itr) {

    hCount->SetBinContent(i + 1, itr->second->GetEntries());
    hCount->GetXaxis()->SetBinLabel(i + 1, itr->first.c_str());

    itr->second->SetLineColor(TColor::GetColor(fColors[i].c_str()));
    itr->second->SetLineWidth(2);

    l->AddEntry(itr->second, Form("#color[%d]{#splitline{%s}{#mu = %.3f, RMS = %.3f}}", TColor::GetColor(fColors[i].c_str()), itr->first.c_str(), itr->second->GetMean(), itr->second->GetRMS()), "l");

    itr->second->SetStats(0);

    c->cd();
    if (i == 0) itr->second->Draw("Hist");
    else itr->second->Draw("Hist same");

    i++;
  }

  c->cd();
  l->Draw();

  TFile* fOutput = new TFile("cputime.root", "RECREATE");
  hCount->Write();
  c->Write();
  fOutput->Close();

  c->SaveAs("cputime.png");

  TCanvas* c2 = new TCanvas("c2","");

  c2->cd();
  hCount->Draw("Hist");
  c2->SaveAs("cpu_count.png");


  return 0;
}
