#include "action_initialization.hh"
#include "primary_generator_action.hh"
#include "run_action.hh"
#include "event_action.hh"
#include "stepping_action.hh"
#include "detector_construction.hh"

#include "G4RunManager.hh"

ActionInitialization::ActionInitialization() {}
ActionInitialization::~ActionInitialization() {}

void ActionInitialization::BuildForMaster() const
{
    RunAction* runAction = new RunAction();
    SetUserAction(runAction);
}

void ActionInitialization::Build() const
{
    SetUserAction(new PrimaryGeneratorAction());

    RunAction* runAction = new RunAction();
    SetUserAction(runAction);

    EventAction* eventAction = new EventAction(runAction);
    SetUserAction(eventAction);

    const DetectorConstruction* detConstruction =
        static_cast<const DetectorConstruction*>(
            G4RunManager::GetRunManager()->GetUserDetectorConstruction());

    SetUserAction(new SteppingAction(detConstruction, runAction));
}
