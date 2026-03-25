#ifndef RUN_ACTION_HH
#define RUN_ACTION_HH

#include "G4UserRunAction.hh"
#include "globals.hh"

#include <vector>
#include <mutex>

class G4Run;

class RunAction : public G4UserRunAction
{
public:
    RunAction();
    ~RunAction() override;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    void AddEdep(G4double edep);

    // Co-60: highest gamma is 1.3325 MeV; set Emax to 1.6 MeV to capture
    // full photopeak + any coincidence sum peak (~2.5 MeV region excluded here).
    // Increase fNbins for better energy resolution at these higher energies.
    static const G4int        fNbins = 800;
    static constexpr G4double fEmax  = 1.6; // MeV

private:
    // Per-worker thread local histogram
    std::vector<G4double> fHistogram;

    // Master histogram merged from all threads
    static std::vector<G4double> fGlobalHistogram;
    static std::mutex             fMutex;
};

#endif
