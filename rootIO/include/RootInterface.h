#ifndef RootInterface_h
#define RootInterface_h 1

#include "TFile.h"
#include "TTree.h"
#include "TChain.h"

template <typename T>

class RootInterface {
public:
  RootInterface(const std::string& filename, bool key);
  ~RootInterface();

  void fill(const T* evt);
  void GetChain(const std::string& treename);
  void read(T& evt);
  void create(const std::string& name, const std::string& title);
  void set(const std::string& name, const std::string& title);
  void write();
  void close();

  TTree* getTree();
  unsigned int entries() { return fTree->GetEntries(); }
  unsigned int numEvt() { return fNumEvt; }

private:
  void init();
  void PrepareChain();

  TChain* fChain;
  TFile* fFile;
  TTree* fTree;
  std::string fFilename;
  T* fEventData;
  unsigned int fNumEvt;
};

#endif
