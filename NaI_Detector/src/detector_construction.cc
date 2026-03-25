#include "detector_construction.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4Material.hh"
#include "G4Element.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    G4NistManager* nist = G4NistManager::Instance();

    // ---------------------------------------------------------------
    // World: air box 30 cm cube
    // ---------------------------------------------------------------
    G4Material* worldMat = nist->FindOrBuildMaterial("G4_AIR");

    G4Box* solidWorld = new G4Box("World", 15.*cm, 15.*cm, 15.*cm);
    G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, worldMat, "World");
    G4VPhysicalVolume* physWorld = new G4PVPlacement(
        nullptr, G4ThreeVector(), logicWorld, "World", nullptr, false, 0, true);

    // ---------------------------------------------------------------
    // NaI(Tl) crystal: cylinder, diameter 7.62 cm (3"), length 7.62 cm
    // Standard 3"x3" NaI detector
    // ---------------------------------------------------------------
    // Build NaI material manually (NIST G4_SODIUM_IODIDE)
    G4Material* naiMat = nist->FindOrBuildMaterial("G4_SODIUM_IODIDE");

    G4double crystalRadius = 3.81*cm;   // 3" diameter => 1.5" radius
    G4double crystalLength = 7.62*cm;   // 3" length

    // Source is 7.5 cm from the FRONT FACE of the detector.
    // Place crystal centre at z = crystalLength/2 = 3.81 cm
    // so front face is at z = 0, source at z = -7.5 cm

    G4Tubs* solidNaI = new G4Tubs("NaI",
                                   0., crystalRadius,
                                   crystalLength / 2.,
                                   0., 360.*deg);

    G4LogicalVolume* logicNaI = new G4LogicalVolume(solidNaI, naiMat, "NaI");

    G4double crystalZ = crystalLength / 2.;   // front face at z=0
    new G4PVPlacement(nullptr,
                      G4ThreeVector(0., 0., crystalZ),
                      logicNaI, "NaI", logicWorld, false, 0, true);

    // Vis attributes
    G4VisAttributes* naiVis = new G4VisAttributes(G4Colour(0.0, 0.8, 0.8, 0.5));
    naiVis->SetVisibility(true);
    naiVis->SetForceSolid(true);
    logicNaI->SetVisAttributes(naiVis);

    G4VisAttributes* worldVis = new G4VisAttributes(G4Colour(1,1,1,0.1));
    worldVis->SetVisibility(false);
    logicWorld->SetVisAttributes(worldVis);

    // Scoring volume = NaI crystal
    fScoringVolume = logicNaI;

    return physWorld;
}
