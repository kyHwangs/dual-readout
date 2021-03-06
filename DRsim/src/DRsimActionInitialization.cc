#include "DRsimActionInitialization.hh"
#include "DRsimPrimaryGeneratorAction.hh"
#include "DRsimRunAction.hh"
#include "DRsimEventAction.hh"
#include "SimG4DRcaloSteppingAction.h"

#include "G4GenericMessenger.hh"

using namespace std;
DRsimActionInitialization::DRsimActionInitialization(G4int seed, G4String filename)
: G4VUserActionInitialization()
{
  fSeed = seed;
  fFilename = filename;

  // G4GenericMessenger::Command::SetDefaultValue does not work on the master thread, remain uninitialized
  fUseHepMC = false;
  fUseCalib = false;
  fUseGPS = false;

  DefineCommands();
}

DRsimActionInitialization::~DRsimActionInitialization() {
  if (fMessenger) delete fMessenger;
}

void DRsimActionInitialization::BuildForMaster() const {
  SetUserAction(new DRsimRunAction(fSeed,fFilename,fUseHepMC));
}

void DRsimActionInitialization::Build() const {
  SetUserAction(new DRsimPrimaryGeneratorAction(fSeed,fUseHepMC,fUseCalib,fUseGPS));

  DRsimRunAction* runAction = new DRsimRunAction(fSeed,fFilename,fUseHepMC);
  SetUserAction(runAction);

  DRsimEventAction* eventAction = new DRsimEventAction();
  SetUserAction(eventAction);

  SimG4DRcaloSteppingAction* steppingAction = new SimG4DRcaloSteppingAction();
  SetUserAction(steppingAction);

  // cheat to avoid constness
  runAction->SetEventAction(eventAction);
  runAction->SetSteppingAction(steppingAction);
}

void DRsimActionInitialization::DefineCommands() {
  fMessenger = new G4GenericMessenger(this, "/DRsim/action/", "action initialization control");
  G4GenericMessenger::Command& ioCmd = fMessenger->DeclareProperty("useHepMC",fUseHepMC,"use HepMC");
  ioCmd.SetParameterName("useHepMC",true,true);

  G4GenericMessenger::Command& calibCmd = fMessenger->DeclareProperty("useCalib",fUseCalib,"use Calib");
  calibCmd.SetParameterName("useCalib",true,true);

  G4GenericMessenger::Command& gpsCmd = fMessenger->DeclareProperty("useGPS",fUseGPS,"use GPS");
  gpsCmd.SetParameterName("useGPS",true,true);
}
