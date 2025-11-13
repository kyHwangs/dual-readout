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

  G4double tempTowerX[92] = {
    3050.15, 3049.97, 3049.18, 3047.78, 3045.78,
    3043.17, 3039.96, 3036.17, 3031.8, 3026.87,
    3021.39, 3015.37, 3008.82, 3001.77, 2994.23,
    2986.21, 2977.74, 2968.83, 2959.51, 2949.78,
    2939.69, 2929.22, 2918.43, 2907.31, 2895.9,
    2884.22, 2872.28, 2860.11, 2847.72, 2835.14,
    2822.38, 2809.46, 2796.41, 2783.23, 2769.95,
    2756.59, 2743.16, 2729.66, 2716.14, 2702.59,
    2689.03, 2675.47, 2661.94, 2648.43, 2634.95,
    2621.53, 2608.17, 2594.88, 2581.68, 2568.55,
    2555.53, 2542.6, 2496.81, 2450.61, 2404.01,
    2357.01, 2309.63, 2261.87, 2213.74, 2165.24,
    2116.39, 2067.2, 2017.66, 1967.8, 1917.61,
    1867.11, 1816.3, 1765.2, 1713.8, 1662.13,
    1610.18, 1557.97, 1505.5, 1452.79, 1399.84,
    1346.66, 1293.26, 1239.64, 1185.83, 1131.82,
    1077.62, 1023.25, 968.709, 914.009, 859.161,
    804.172, 749.051, 693.807, 638.449, 582.987,
    527.43, 471.786
  };
  std::copy(tempTowerX, tempTowerX + 92, fTowerX);

  G4double tempTowerZ[92] = {
    33.8885, 101.663, 169.419, 237.168, 304.905,
    372.609, 440.278, 507.905, 575.487, 643.019,
    710.498, 777.92, 845.282, 912.579, 979.827,
    1047.02, 1114.16, 1181.25, 1248.31, 1315.35,
    1382.37, 1449.38, 1516.4, 1583.42, 1650.48,
    1717.58, 1784.73, 1851.98, 1919.33, 1986.81,
    2054.45, 2122.26, 2190.27, 2258.53, 2327.02,
    2395.78, 2464.87, 2534.29, 2604.09, 2674.29,
    2744.93, 2816.04, 2887.66, 2959.85, 3032.59,
    3105.95, 3179.96, 3254.66, 3330.12, 3406.35,
    3483.4, 3561.32, 3593.58, 3625.24, 3656.31,
    3686.78, 3716.65, 3745.91, 3774.55, 3802.58,
    3829.98, 3856.76, 3882.9, 3908.41, 3933.28,
    3957.5, 3981.07, 4003.99, 4026.26, 4047.87,
    4068.81, 4089.09, 4108.69, 4127.63, 4145.88,
    4163.46, 4180.36, 4196.57, 4212.09, 4226.92,
    4241.06, 4254.51, 4267.26, 4279.31, 4290.66,
    4301.3, 4311.24, 4320.48, 4329, 4336.82,
    4343.93, 4350.32
  };
  std::copy(tempTowerZ, tempTowerZ + 92, fTowerZ);

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

  G4double xBeamCenterCorrected = xBeamCenter + xBeamCeneterToTowerCenter * (rBeamCenterToTowerCenter - 1800.) / rBeamCenterToTowerCenter;
  G4double yBeamCenterCorrected = yBeamCenter + yBeamCeneterToTowerCenter * (rBeamCenterToTowerCenter - 1800.) / rBeamCenterToTowerCenter;
  G4double zBeamCenterCorrected = zBeamCenter + zBeamCeneterToTowerCenter * (rBeamCenterToTowerCenter - 1800.) / rBeamCenterToTowerCenter;

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
