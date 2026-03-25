#include "event_action.hh"
#include "run_action.hh"

#include "G4RunManager.hh"
#include "G4Event.hh"

EventAction::EventAction(RunAction* runAction)
    : G4UserEventAction(), fRunAction(runAction), fEdep(0.0)
{}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event*)
{
    fEdep = 0.0;
}

void EventAction::EndOfEventAction(const G4Event*)
{
    // Record total energy deposited in NaI for this event
    fRunAction->AddEdep(fEdep);
}
