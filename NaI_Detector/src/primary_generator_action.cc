#include "primary_generator_action.hh"

#include "G4GeneralParticleSource.hh"
#include "G4SystemOfUnits.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
    fGPS = new G4GeneralParticleSource();
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fGPS;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    fGPS->GeneratePrimaryVertex(event);
}
