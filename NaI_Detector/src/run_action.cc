#include "run_action.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <numeric>

// Static member definitions
std::vector<G4double> RunAction::fGlobalHistogram(RunAction::fNbins, 0.0);
std::mutex            RunAction::fMutex;

RunAction::RunAction()
    : G4UserRunAction(),
      fHistogram(fNbins, 0.0)
{
    // Reset global histogram when master run action is created
    if (IsMaster()) {
        std::lock_guard<std::mutex> lock(fMutex);
        std::fill(fGlobalHistogram.begin(), fGlobalHistogram.end(), 0.0);
    }
}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*)
{
    // Reset this thread's local histogram
    std::fill(fHistogram.begin(), fHistogram.end(), 0.0);
}

void RunAction::AddEdep(G4double edep)
{
    if (edep <= 0.) return;

    G4int bin = static_cast<G4int>(edep / MeV / fEmax * fNbins);
    if (bin < 0)       bin = 0;
    if (bin >= fNbins) bin = fNbins - 1;

    fHistogram[bin] += 1.0;
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    // Merge this worker's histogram into the global one (thread-safe)
    {
        std::lock_guard<std::mutex> lock(fMutex);
        for (G4int i = 0; i < fNbins; i++) {
            fGlobalHistogram[i] += fHistogram[i];
        }
    }

    // Only master thread writes output
    if (!IsMaster()) return;

    G4int nEvents = run->GetNumberOfEvent();
    G4cout << "\n=== NaI Detector Simulation Complete (Co-60) ===" << G4endl;
    G4cout << "Total events: " << nEvents << G4endl;

    // Try results/ directory first, fall back to current dir
    std::ofstream outFile("results/energy_spectrum_co60.txt");
    if (!outFile.is_open()) {
        outFile.open("energy_spectrum_co60.txt");
    }

    outFile << "# Energy(MeV)  Counts\n";
    G4cout << "\n# Energy(MeV)  Counts" << G4endl;

    G4double binWidth   = fEmax / fNbins;
    G4double totalCounts = 0;

    for (G4int i = 0; i < fNbins; i++) {
        G4double eMeV   = (i + 0.5) * binWidth;
        G4int    counts = static_cast<G4int>(fGlobalHistogram[i]);
        outFile << std::fixed << std::setprecision(4)
                << eMeV << "  " << counts << "\n";
        if (counts > 0) {
            G4cout << std::fixed << std::setprecision(4)
                   << eMeV << "  " << counts << G4endl;
            totalCounts += counts;
        }
    }

    outFile.close();
    G4cout << "\nTotal counts in detector: " << (G4int)totalCounts << G4endl;
    G4cout << "Spectrum written to results/energy_spectrum_co60.txt" << G4endl;
}
