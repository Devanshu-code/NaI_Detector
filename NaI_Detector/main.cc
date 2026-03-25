#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "QBBC.hh"

#include "action_initialization.hh"
#include "detector_construction.hh"

int main(int argc, char** argv)
{
    if (argc != 2) {
        G4cerr << "Usage: nai_detector <macro.mac>" << G4endl;
        return 1;
    }

    auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

    runManager->SetUserInitialization(new DetectorConstruction());

    auto* physicsList = new QBBC;
    physicsList->SetVerboseLevel(1);
    runManager->SetUserInitialization(physicsList);

    runManager->SetUserInitialization(new ActionInitialization());

    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    UImanager->ApplyCommand("/control/execute " + G4String(argv[1]));

    delete runManager;
    return 0;
}
