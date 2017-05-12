#include "SimApplication/PrimaryGeneratorMessenger.h"

// LDMX
#include "SimApplication/LHEPrimaryGenerator.h"
#include "SimApplication/RootPrimaryGenerator.h"
#include "SimApplication/MultiParticleGunPrimaryGenerator.h"

namespace ldmx {

    bool PrimaryGeneratorMessenger::useRootSeed_{false};
    double PrimaryGeneratorMessenger::nParticles_{1.0};
    bool PrimaryGeneratorMessenger::enablePoisson_{false};
    // std::string PrimaryGeneratorMessenger::particleType_{"e-"};
    // double PrimaryGeneratorMessenger::particleEnergy_{4.0};

    PrimaryGeneratorMessenger::PrimaryGeneratorMessenger(PrimaryGeneratorAction* thePrimaryGeneratorAction) :
            primaryGeneratorAction_(thePrimaryGeneratorAction) {

        // lhe commands
        lheDir_ = new G4UIdirectory("/ldmx/generators/lhe/");
        lheDir_->SetGuidance("Commands for LHE event generation");

        lheOpenCmd_ = new G4UIcommand("/ldmx/generators/lhe/open", this);
        G4UIparameter* lhefilename = new G4UIparameter("filename", 's', true);
        lheOpenCmd_->SetParameter(lhefilename);
        lheOpenCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);

        // root commands
        rootDir_ = new G4UIdirectory("/ldmx/generators/root/");
        rootDir_->SetGuidance("Commands for ROOT event generation");

        rootOpenCmd_ = new G4UIcommand("/ldmx/generators/root/open", this);
        G4UIparameter* rootfilename = new G4UIparameter("filename", 's', true);
        rootOpenCmd_->SetParameter(rootfilename);
        rootOpenCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);

        rootUseSeedCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);

    }

    PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger() {;}

    void PrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {

        if      (command == lheOpenCmd_)            { primaryGeneratorAction_->setPrimaryGenerator(new LHEPrimaryGenerator(new LHEReader(newValues))); }
        else if (command == rootOpenCmd_)           { primaryGeneratorAction_->setPrimaryGenerator(new RootPrimaryGenerator(newValues)); }
        else if (command == rootUseSeedCmd_)        { useRootSeed_ = true; }
         
        // else if (command == mpgunEnergyCmd_)        { particleEnergy_ = G4UIcommand::ConvertToDouble(newValues); }
        // else if (command == mpgunParticleTypeCmd_)  { particleType_ = newValues; }
        else if (command == enableMPGunCmd_)        { primaryGeneratorAction_->setPrimaryGenerator(new MultiParticleGunPrimaryGenerator()); }
        else if (command == mpgunNParCmd_)          { nParticles_ = G4UIcommand::ConvertToDouble(newValues); }        
        else if (command == enableMPGunPoissonCmd_) { enablePoisson_ = true; }
 
        else if (command == enableBeamspotCmd_)     { primaryGeneratorAction_->setUseBeamspot(true); }
        else if (command == beamspotXSizeCmd_)      { primaryGeneratorAction_->setBeamspotXSize( G4UIcommand::ConvertToDouble(newValues) ); }
        else if (command == beamspotYSizeCmd_)      { primaryGeneratorAction_->setBeamspotYSize( G4UIcommand::ConvertToDouble(newValues) ); }
    }

}
