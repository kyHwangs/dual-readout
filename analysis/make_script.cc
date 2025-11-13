#include <iostream>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <array>
#include <fstream>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
  std::string fConfigName = argv[1];

  const char* fBaseDirTemp = getenv("DRCBASE");
  if (fBaseDirTemp == nullptr) {
    std::cout << "ENV set is needed before running the script" << std::endl;
    return -1;
  }

  std::string fBaseDir = std::string(fBaseDirTemp);

  std::string fInstallBaseDir = fBaseDir + "/install";
  std::string fConfigBaseDir = fBaseDir + "/install/" + fConfigName;
  std::string fRootBaseDir = fConfigBaseDir + "/root";
  std::string fLogBaseDir = fConfigBaseDir + "/log";

  fs::path fConfigDir(fConfigBaseDir.data());
  if( !(fs::exists(fConfigDir)) ) fs::create_directory(fConfigDir);

  fs::path fRootDir(fRootBaseDir.data());
  if( !(fs::exists(fRootDir)) ) fs::create_directory(fRootDir);

  fs::path fLogDir(fLogBaseDir.data());
  if( !(fs::exists(fLogDir)) ) fs::create_directory(fLogDir);

  std::string fRunMacro = R"(#This macro can not be executed standalone
/vis/disable
/run/numberOfThreads 1
/run/initialize
/run/verbose 1

/DRsim/generator/randy 10.
/DRsim/generator/randz 10.
/DRsim/generator/nTower 0

/gun/particle e-
/gun/energy 20 GeV

/run/beamOn 2
)";

  std::string fMacro = fConfigBaseDir + "/run_macro.mac";
  std::ofstream fMacroStream(fMacro);
  if (fMacroStream.is_open()) {
    fMacroStream << fRunMacro;
    fMacroStream.close();
  } else {
    std::cout << "Failed to create config file: " << fMacro << std::endl;
    return -1;
  }
  
  std::string fCondorExcutable = R"(#! /bin/sh
cd )" + fBaseDir + R"(
export PATH=/cvmfs/sft.cern.ch/lcg/contrib/CMake/3.14.2/Linux-x86_64/bin:$PATH
source /cvmfs/sft.cern.ch/lcg/contrib/gcc/8/x86_64-centos7/setup.sh

source /cvmfs/sft.cern.ch/lcg/releases/LCG_96b/ROOT/6.18.04/x86_64-centos7-gcc8-opt/ROOT-env.sh
source /cvmfs/geant4.cern.ch/geant4/10.5.p01/x86_64-centos7-gcc8-opt-MT/CMake-setup.sh

export HEPMC_DIR=/cvmfs/sft.cern.ch/lcg/releases/LCG_96b/hepmc3/3.1.2/x86_64-centos7-gcc8-opt
export FASTJET_DIR=/cvmfs/sft.cern.ch/lcg/releases/LCG_96b/fastjet/3.3.2/x86_64-centos7-gcc8-opt
export PYTHIA_DIR=/cvmfs/sft.cern.ch/lcg/releases/LCG_96b/MCGenerators/pythia8/240/x86_64-centos7-gcc8-opt

export PYTHIA8=/cvmfs/sft.cern.ch/lcg/releases/LCG_96b/MCGenerators/pythia8/240/x86_64-centos7-gcc8-opt
export PYTHIA8DATA=/cvmfs/sft.cern.ch/lcg/releases/LCG_96b/MCGenerators/pythia8/240/x86_64-centos7-gcc8-opt/share/Pythia8/xmldoc
export ROOT_INCLUDE_PATH=/cvmfs/sft.cern.ch/lcg/releases/LCG_96b/hepmc3/3.1.2/x86_64-centos7-gcc8-opt/include:$ROOT_INCLUDE_PATH

export DRCBASE=`pwd`
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HEPMC_DIR/lib64:$FASTJET_DIR/lib:$PYTHIA_DIR:$PWD/install/lib
cd )" + fConfigBaseDir + R"(

)" + fInstallBaseDir + R"(/bin/DRsim \
)" + fConfigBaseDir + R"(/run_macro.mac \
$1 )" + fRootBaseDir + R"(/output
)";  

  std::string fConfigWrapper = fConfigBaseDir + "/condor_wraper.sh";
  std::ofstream fConfigWrapperStream(fConfigWrapper);
  if (fConfigWrapperStream.is_open()) {
    fConfigWrapperStream << fCondorExcutable;
    fConfigWrapperStream.close();
  } else {
    std::cout << "Failed to create config file: " << fConfigWrapper << std::endl;
    return -1;
  }

  std::string fCondorSubmit = R"(universe            = vanilla
executable          = condor_wraper.sh
arguments           = $(ProcId)
output              = )" + fLogBaseDir + R"(/out_$(ProcId).out
error               = )" + fLogBaseDir + R"(/err_$(ProcId).err
log                 = )" + fLogBaseDir + R"(/log_$(ProcId).log
request_memory      = 1.5 GB

should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = )" + fInstallBaseDir + R"(/bin, \
                       )" + fInstallBaseDir + R"(/lib, \
                       )" + fInstallBaseDir + R"(/init.mac, \
                       )" + fConfigBaseDir + R"(/run_macro.mac

JobBatchName = )" + fConfigName + R"(

queue 500)";

  std::string fConfigSubmit = fConfigBaseDir + "/condor_submit.sub";
  std::ofstream fConfigSubmitStream(fConfigSubmit);
  if (fConfigSubmitStream.is_open()) {
    fConfigSubmitStream << fCondorSubmit;
    fConfigSubmitStream.close();
  } else {
    std::cout << "Failed to create config file: " << fCondorSubmit << std::endl;
    return -1;
  }

  return 0;
}