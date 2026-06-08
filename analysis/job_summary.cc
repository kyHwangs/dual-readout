#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>

#include "TFile.h"
#include "TTree.h"
#include "TROOT.h"
#include "TError.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <nFiles> [startIndex=0]" << std::endl;
    return 1;
  }

  int nFiles = std::stoi(argv[1]);
  std::string fInputBase = (argc >= 3) ? argv[2] : "./root/output_";
  int startIdx = (argc >= 4) ? std::stoi(argv[3]) : 0;

  // Suppress ROOT's verbose warnings; only show errors and above
  gErrorIgnoreLevel = kError;

  std::ofstream jobList("job_list.txt");
  if (!jobList.is_open()) {
    std::cerr << "Failed to open job_list.txt for writing" << std::endl;
    return 1;
  }

  int nGood = 0, nBad = 0;

  for (int i = startIdx; i < startIdx + nFiles; i++) {
    std::string filename = fInputBase + std::to_string(i) + ".root";

    TFile* f = TFile::Open(filename.c_str(), "READ");

    // IsZombie() is true when TFile::Open fails (file missing, permission denied, etc.)
    auto removeBadFile = [&](const std::string& fname) {
      jobList << i << "\n";
      nBad++;
      if (std::remove(fname.c_str()) == 0)
        std::cout << "       -> deleted " << fname << std::endl;
      else
        std::cout << "       -> failed to delete " << fname << std::endl;
    };

    if (!f || f->IsZombie() || !f->IsOpen()) {
      std::cout << "[BAD] " << filename << " (failed to open)" << std::endl;
      if (f) {
        // Zombie files must be removed from ROOT's internal list before delete;
        // calling Close() on them can crash because their internal state is invalid.
        gROOT->GetListOfFiles()->Remove(f);
        delete f;
      }
      removeBadFile(filename);
      continue;
    }

    // Files that were not properly closed by the simulation have no keys
    if (f->GetNkeys() == 0) {
      std::cout << "[BAD] " << filename << " (no keys - simulation did not close properly)" << std::endl;
      f->Close();
      delete f;
      removeBadFile(filename);
      continue;
    }

    // Check that the expected TTree exists
    TTree* tree = dynamic_cast<TTree*>(f->Get("DRsim"));
    if (!tree) {
      std::cout << "[BAD] " << filename << " (TTree 'DRsim' not found, keys: " << f->GetNkeys() << ")" << std::endl;
      f->Close();
      delete f;
      removeBadFile(filename);
      continue;
    }

    std::cout << "[OK]  " << filename << " (" << tree->GetEntries() << " events)" << std::endl;
    f->Close();
    delete f;
    nGood++;
  }

  jobList.close();

  std::cout << "\n=== Summary ===" << std::endl;
  std::cout << "Good: " << nGood << " / " << nFiles << std::endl;
  std::cout << "Bad:  " << nBad  << " / " << nFiles << std::endl;
  if (nBad > 0)
    std::cout << "Bad file indices saved to job_list.txt" << std::endl;

  return 0;
}
