#!/bin/sh

source /cvmfs/sft.cern.ch/lcg/views/LCG_107/x86_64-el9-gcc13-opt/setup.sh

export HEPMC_DIR=/cvmfs/sft.cern.ch/lcg/releases/hepmc3/3.2.7-cd228/x86_64-el9-gcc13-opt
export FASTJET_DIR=/cvmfs/sft.cern.ch/lcg/releases/fastjet/3.4.3-7b8bd/x86_64-el9-gcc13-opt
export PYTHIA_DIR=/cvmfs/sft.cern.ch/lcg/releases/MCGenerators/pythia8/313-ba28f/x86_64-el9-gcc13-opt

export PYTHIA8=/cvmfs/sft.cern.ch/lcg/releases/MCGenerators/pythia8/313-ba28f/x86_64-el9-gcc13-opt
export PYTHIA8DATA=/cvmfs/sft.cern.ch/lcg/releases/MCGenerators/pythia8/313-ba28f/x86_64-el9-gcc13-opt/share/Pythia8/xmldoc
export ROOT_INCLUDE_PATH=/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.34.02-18eb6/x86_64-el9-gcc13-opt/include:$ROOT_INCLUDE_PATH

export DRCBASE=`pwd`
alias build_drc='cd $DRCBASE/build; make -j4 install; cd -'

export INSTALL_DIR_PATH=$PWD/install
export PATH=$PATH:$INSTALL_DIR_PATH/lib:$INSTALL_DIR_PATH/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INSTALL_DIR_PATH/lib

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HEPMC_DIR/lib64:$FASTJET_DIR/lib:$PYTHIA_DIR/lib
