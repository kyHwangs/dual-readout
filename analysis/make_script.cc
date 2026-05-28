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

/run/numberOfThreads 1
/run/initialize
/run/verbose 1

/DRsim/generator/x0 0
/DRsim/generator/y0 0
/DRsim/generator/randx 10.
/DRsim/generator/randy 10.

/gun/particle e-
/gun/energy 20 GeV

/run/beamOn 1
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
  
  std::string fCondorExcutable = R"(#! /bin/bash

lscpu | grep "Model name"

source /cvmfs/sft.cern.ch/lcg/views/LCG_107/x86_64-el9-gcc13-opt/setup.sh

export HEPMC_DIR=/cvmfs/sft.cern.ch/lcg/releases/hepmc3/3.2.7-cd228/x86_64-el9-gcc13-opt
export FASTJET_DIR=/cvmfs/sft.cern.ch/lcg/releases/fastjet/3.4.3-7b8bd/x86_64-el9-gcc13-opt
export PYTHIA_DIR=/cvmfs/sft.cern.ch/lcg/releases/MCGenerators/pythia8/313-ba28f/x86_64-el9-gcc13-opt

export PYTHIA8=/cvmfs/sft.cern.ch/lcg/releases/MCGenerators/pythia8/313-ba28f/x86_64-el9-gcc13-opt
export PYTHIA8DATA=/cvmfs/sft.cern.ch/lcg/releases/MCGenerators/pythia8/313-ba28f/x86_64-el9-gcc13-opt/share/Pythia8/xmldoc
export ROOT_INCLUDE_PATH=/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.34.02-18eb6/x86_64-el9-gcc13-opt/include:$ROOT_INCLUDE_PATH

export INSTALL_DIR_PATH=$PWD
export PATH=$PATH:$INSTALL_DIR_PATH/lib:$INSTALL_DIR_PATH/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INSTALL_DIR_PATH/lib

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HEPMC_DIR/lib64:$FASTJET_DIR/lib:$PYTHIA_DIR/lib

export HOME="${_CONDOR_SCRATCH_DIR:-$PWD}"

./bin/DRsim \
run_macro.mac \
$1 output
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
ProcOffset          = $(Process)
ProcOffsetInt       = $INT(ProcOffset)
arguments           = $(ProcOffsetInt)
output              = )" + fLogBaseDir + R"(/out_$(ProcOffsetInt).out
error               = )" + fLogBaseDir + R"(/err_$(ProcOffsetInt).err
log                 = )" + fLogBaseDir + R"(/log_$(ProcOffsetInt).log
request_memory      = 300 MB

should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = )" + fInstallBaseDir + R"(/bin, \
                       )" + fInstallBaseDir + R"(/lib, \
                       )" + fInstallBaseDir + R"(/init.mac, \
                       )" + fConfigBaseDir + R"(/run_macro.mac

JobBatchName = )" + fConfigName + R"(

queue 3000)";

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
