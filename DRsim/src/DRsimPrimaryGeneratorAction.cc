#include "DRsimPrimaryGeneratorAction.hh"
#include "DRsimRunAction.hh"

#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4GenericMessenger.hh"
#include "G4SystemOfUnits.hh"
#include "G4AutoLock.hh"
#include "Randomize.hh"
#include <cmath>
#include <algorithm>

namespace { G4Mutex DRsimPrimaryGeneratorMutex = G4MUTEX_INITIALIZER; }
int DRsimPrimaryGeneratorAction::sNumEvt = 0;
G4ThreadLocal int DRsimPrimaryGeneratorAction::sIdxEvt = 0;

using namespace std;
DRsimPrimaryGeneratorAction::DRsimPrimaryGeneratorAction(G4int seed, G4bool useHepMC, G4bool useCalib, G4bool useGPS)
: G4VUserPrimaryGeneratorAction()
{
  fSeed = seed;
  fUseHepMC = useHepMC;
  fUseCalib = useCalib;
  fUseGPS = useGPS;

  G4double tempTowerX[80] = {
    1549.96, 1549.42, 1548.26, 1546.49, 1544.12, 
    1541.14, 1537.57, 1533.42, 1528.69, 1523.41, 
    1517.57, 1511.2, 1504.31, 1496.92, 1489.05, 
    1480.7, 1471.9, 1462.67, 1453.03, 1443, 
    1432.59, 1421.83, 1410.74, 1399.34, 1387.65, 
    1375.69, 1363.49, 1351.05, 1338.41, 1325.58, 
    1312.57, 1299.42, 1286.13, 1272.73, 1259.23, 
    1245.66, 1232.01, 1218.33, 1204.61, 1190.87, 
    1168.46, 1145.28, 1122.17, 1099.09, 1076.05, 
    1053.03, 1030.02, 1007.01, 984.003, 960.986, 
    937.954, 914.903, 891.826, 868.721, 845.584, 
    822.412, 799.202, 775.952, 752.661, 729.326, 
    705.946, 682.522, 659.052, 635.535, 611.972, 
    588.364, 564.709, 541.01, 517.265, 493.478, 
    469.648, 445.777, 421.866, 397.917, 373.931, 
    349.91, 325.856, 301.771, 277.656, 253.514
  };
  std::copy(tempTowerX, tempTowerX + 80, fTowerX);

  G4double tempTowerZ[80] = {
    17.2208, 51.6457, 86.0245, 120.343, 154.577, 
    188.699, 222.686, 256.518, 290.172, 323.628, 
    356.866, 389.868, 422.614, 455.085, 487.273, 
    519.159, 550.729, 581.973, 612.886, 643.452, 
    673.667, 703.524, 733.014, 762.131, 790.872, 
    819.237, 847.222, 874.835, 902.073, 928.939, 
    955.44, 981.575, 1007.36, 1032.79, 1057.87, 
    1082.61, 1107.03, 1131.13, 1154.91, 1178.4, 
    1193.03, 1206.63, 1220, 1233.15, 1246.07, 
    1258.75, 1271.2, 1283.41, 1295.38, 1307.11, 
    1318.59, 1329.82, 1340.79, 1351.51, 1361.97, 
    1372.17, 1382.11, 1391.78, 1401.19, 1410.32, 
    1419.18, 1427.77, 1436.08, 1444.11, 1451.86, 
    1459.32, 1466.5, 1473.4, 1480.01, 1486.32, 
    1492.35, 1498.08, 1503.52, 1508.67, 1513.51, 
    1518.06, 1522.31, 1526.26, 1529.91, 1533.26
  };
  std::copy(tempTowerZ, tempTowerZ + 80, fTowerZ);

  if (!fUseHepMC) {
    if (fUseGPS) initGPS();
    else initPtcGun();
  }
}

void DRsimPrimaryGeneratorAction::initPtcGun() {
  fTheta = -0.01111;
  fPhi = 0.;
  fRandX = 10.*mm;
  fRandY = 10.*mm;
  fY_0 = 0.;
  fZ_0 = 0.;
  fParticleGun = new G4ParticleGun(1);

  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName;
  fElectron = particleTable->FindParticle(particleName="e-");
  fPositron = particleTable->FindParticle(particleName="e+");
  fMuon = particleTable->FindParticle(particleName="mu+");
  fPion = particleTable->FindParticle(particleName="pi+");
  fKaon0L = particleTable->FindParticle(particleName="kaon0L");
  fProton = particleTable->FindParticle(particleName="proton");
  fOptGamma = particleTable->FindParticle(particleName="opticalphoton");

  // define commands for this class
  DefineCommands();
}

void DRsimPrimaryGeneratorAction::initGPS() {
  fGPS = new G4GeneralParticleSource();
}

DRsimPrimaryGeneratorAction::~DRsimPrimaryGeneratorAction() {
  if (!fUseHepMC) {
    if (fUseGPS) delete fGPS;
    else {
      if (fParticleGun) delete fParticleGun;
      if (fMessenger) delete fMessenger;
    }
  }
}

void DRsimPrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {

  if (fUseGPS) {
    G4AutoLock lock(&DRsimPrimaryGeneratorMutex);
    fGPS->GeneratePrimaryVertex(event);
    sIdxEvt = sNumEvt;
    sNumEvt++;

    return;
  }

  if (fUseHepMC) {
    G4AutoLock lock(&DRsimPrimaryGeneratorMutex);
    DRsimRunAction::sHepMCreader->GeneratePrimaryVertex(event);
    sIdxEvt = sNumEvt;
    sNumEvt++;

    return;
  }

  G4double randy = G4UniformRand() * fRandY * mm - fRandY * mm / 2.;
  G4double randz = G4UniformRand() * fRandZ * mm - fRandZ * mm / 2.;

  G4double x0 = fTowerX[fTowerNum];
  G4double y0 = 0.;
  G4double z0 = fTowerZ[fTowerNum];

  G4double r0 = sqrt(x0*x0 + y0*y0 + z0*z0);

  x0 = x0 * (r0 - 1250.) / r0;
  y0 = y0 * (r0 - 1250.) / r0;
  z0 = z0 * (r0 - 1250.) / r0;

  G4double thetacenter = atan(z0 / x0);

  G4double z0p = x0 * tan(thetacenter + 1.5 * M_PI / 180.);

  G4double dz0 = z0p - z0;
  G4double dy0 = x0 * tan(1.0 * M_PI / 180.);

  G4double xBeamCenter = 0.;
  G4double yBeamCenter = -dy0 *mm;
  G4double zBeamCenter = -dz0 *mm;

  G4double xBeamCeneterToTowerCenter = x0 - xBeamCenter;
  G4double yBeamCeneterToTowerCenter = y0 - yBeamCenter;
  G4double zBeamCeneterToTowerCenter = z0 - zBeamCenter;

  G4double rBeamCenterToTowerCenter = sqrt(xBeamCeneterToTowerCenter*xBeamCeneterToTowerCenter + yBeamCeneterToTowerCenter*yBeamCeneterToTowerCenter + zBeamCeneterToTowerCenter*zBeamCeneterToTowerCenter);

  G4double xBeamCenterCorrected = xBeamCenter + xBeamCeneterToTowerCenter * (rBeamCenterToTowerCenter - 300.) / rBeamCenterToTowerCenter;
  G4double yBeamCenterCorrected = yBeamCenter + yBeamCeneterToTowerCenter * (rBeamCenterToTowerCenter - 300.) / rBeamCenterToTowerCenter;
  G4double zBeamCenterCorrected = zBeamCenter + zBeamCeneterToTowerCenter * (rBeamCenterToTowerCenter - 300.) / rBeamCenterToTowerCenter;

  auto GunPosition = new G4ThreeVector(0., randy, randz);
  GunPosition->rotateY(fTheta);

  fOrg.set(xBeamCenterCorrected - GunPosition->getX() *mm, yBeamCenterCorrected - GunPosition->getY() *mm, zBeamCenterCorrected - GunPosition->getZ() *mm);
  
  fParticleGun->SetParticlePosition(fOrg); // http://www.apc.univ-paris7.fr/~franco/g4doxy/html/classG4VPrimaryGenerator.html

  fDirection.setREtaPhi(1.,0.,0.);
  fDirection.rotateY(- thetacenter - 1.5 * M_PI / 180.);
  fDirection.rotateZ(1.0 * M_PI / 180.);

  fParticleGun->SetParticleMomentumDirection(fDirection);

  G4AutoLock lock(&DRsimPrimaryGeneratorMutex);
  fParticleGun->GeneratePrimaryVertex(event);
  sIdxEvt = sNumEvt;
  sNumEvt++;

  return;
}

void DRsimPrimaryGeneratorAction::DefineCommands() {
  // Define /DRsim/generator command directory using generic messenger class
  fMessenger = new G4GenericMessenger(this, "/DRsim/generator/", "Primary generator control");

  G4GenericMessenger::Command& numCmd = fMessenger->DeclareMethod("nTower", &DRsimPrimaryGeneratorAction::SetTowerNum,"number of the tower");
  numCmd.SetParameterName("nTower",true);
  numCmd.SetDefaultValue("0");

  G4GenericMessenger::Command& etaCmd = fMessenger->DeclareMethodWithUnit("theta","rad",&DRsimPrimaryGeneratorAction::SetTheta,"theta of beam");
  etaCmd.SetParameterName("theta",true);
  etaCmd.SetDefaultValue("-0.01111");

  G4GenericMessenger::Command& phiCmd = fMessenger->DeclareMethodWithUnit("phi","rad",&DRsimPrimaryGeneratorAction::SetPhi,"phi of beam");
  phiCmd.SetParameterName("phi",true);
  phiCmd.SetDefaultValue("0.");

  G4GenericMessenger::Command& y0Cmd = fMessenger->DeclareMethodWithUnit("y0","cm",&DRsimPrimaryGeneratorAction::SetY0,"y_0 of beam");
  y0Cmd.SetParameterName("y0",true);
  y0Cmd.SetDefaultValue("0.");

  G4GenericMessenger::Command& z0Cmd = fMessenger->DeclareMethodWithUnit("z0","cm",&DRsimPrimaryGeneratorAction::SetZ0,"z_0 of beam");
  z0Cmd.SetParameterName("z0",true);
  z0Cmd.SetDefaultValue("0.");

  G4GenericMessenger::Command& randxCmd = fMessenger->DeclareMethodWithUnit("randx","mm",&DRsimPrimaryGeneratorAction::SetRandX,"x width of beam");
  randxCmd.SetParameterName("randx",true);
  randxCmd.SetDefaultValue("0.");

  G4GenericMessenger::Command& randyCmd = fMessenger->DeclareMethodWithUnit("randy","mm",&DRsimPrimaryGeneratorAction::SetRandY,"y width of beam");
  randyCmd.SetParameterName("randy",true);
  randyCmd.SetDefaultValue("3.");

  G4GenericMessenger::Command& randzCmd = fMessenger->DeclareMethodWithUnit("randz","mm",&DRsimPrimaryGeneratorAction::SetRandZ,"z width of beam");
  randzCmd.SetParameterName("randz",true);
  randzCmd.SetDefaultValue("3.");
}
