#ifndef STEPPING_ACTION_HH
#define STEPPING_ACTION_HH

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class DetectorConstruction;
class RunAction;

class SteppingAction : public G4UserSteppingAction
{
public:
    SteppingAction(const DetectorConstruction* detConstruction,
                   RunAction* runAction);
    ~SteppingAction() override;

    void UserSteppingAction(const G4Step* step) override;

private:
    const DetectorConstruction* fDetConstruction;
    RunAction* fRunAction;
};

#endif
