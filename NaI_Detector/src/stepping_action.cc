#include "stepping_action.hh"
#include "detector_construction.hh"
#include "event_action.hh"

#include "G4Step.hh"
#include "G4RunManager.hh"
#include "G4LogicalVolume.hh"

SteppingAction::SteppingAction(const DetectorConstruction* detConstruction,
                               RunAction* runAction)
    : G4UserSteppingAction(),
      fDetConstruction(detConstruction),
      fRunAction(runAction)
{}

SteppingAction::~SteppingAction() {}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    // Get scoring volume
    const G4LogicalVolume* volume =
        step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume();

    if (volume != fDetConstruction->GetScoringVolume()) return;

    // Accumulate energy deposit in NaI
    G4double edep = step->GetTotalEnergyDeposit();

    // Access event action via run manager
    EventAction* eventAction = const_cast<EventAction*>(
        static_cast<const EventAction*>(
            G4RunManager::GetRunManager()->GetUserEventAction()));

    if (eventAction) eventAction->AddEdep(edep);
}
