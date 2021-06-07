#include <iostream>

#include "TFile.h"
#include "TString.h"
#include "TTree.h"

#include "../include/EventInitialState.h"
#include "../include/UEvent.h"
#include "../include/URun.h"

R__LOAD_LIBRARY(libMcIniData.so)

void convertSmashParticles(TString inputFileName = "particles.root", TString outputFileName = "mcini_smash.root", Bool_t only_not_empty = true,
    Int_t aProj = 197, Int_t zProj = 79, Double_t pProj = 1.955, 
    Int_t aTarg = 197, Int_t zTarg = 79, Double_t pTarg = 0.,
    Double_t bMin = 0., Double_t bMax = 16., Int_t bWeight = 0,
    Double_t phiMin = 0., Double_t phiMax = 0.,
    Double_t sigma = 0., Int_t nEvents = 0)
{
    TFile *fIn = new TFile(inputFileName, "read");
    TTree *fTree = (TTree*) fIn->Get("particles");
    if (!fTree)
    {
        std::cerr << "TTree was not found in the input file." << std::endl;
        return;
    }

    Int_t t_ev;
    Int_t t_tcounter;
    Int_t t_npart;
    Int_t t_test_p;
    Double_t t_modus_l;
    Double_t t_current_t;
    Double_t t_impact_b;
    Bool_t t_empty_event;

    static const int max_npart = 25000;

    Int_t    t_pdgcode[max_npart];
    Int_t    t_charge[max_npart];
    Double_t t_p0[max_npart];
    Double_t t_px[max_npart];
    Double_t t_py[max_npart];
    Double_t t_pz[max_npart];
    Double_t t_t[max_npart];
    Double_t t_x[max_npart];
    Double_t t_y[max_npart];
    Double_t t_z[max_npart];

    // Double_t t_E_kinetic_tot;
    // Double_t t_E_fields_tot;
    // Double_t t_E_tot;

    fTree->SetBranchAddress("ev",&t_ev);
    fTree->SetBranchAddress("tcounter",&t_tcounter);
    fTree->SetBranchAddress("npart",&t_npart);
    fTree->SetBranchAddress("test_p",&t_test_p);
    fTree->SetBranchAddress("modus_l",&t_modus_l);
    fTree->SetBranchAddress("current_t",&t_current_t);
    fTree->SetBranchAddress("impact_b",&t_impact_b);
    fTree->SetBranchAddress("empty_event",&t_empty_event);
    fTree->SetBranchAddress("pdgcode",t_pdgcode);
    fTree->SetBranchAddress("charge",t_charge);
    fTree->SetBranchAddress("p0",t_p0);
    fTree->SetBranchAddress("px",t_px);
    fTree->SetBranchAddress("py",t_py);
    fTree->SetBranchAddress("pz",t_pz);
    fTree->SetBranchAddress("t",t_t);
    fTree->SetBranchAddress("x",t_x);
    fTree->SetBranchAddress("y",t_y);
    fTree->SetBranchAddress("z",t_z);

    TFile *fOut = new TFile(outputFileName, "recreate");

    TTree *iniTree = new TTree("events", "SMASH");
    URun header("SMASH","final state only", aProj, zProj, pProj, aTarg, zTarg, pTarg, bMin, bMax, bWeight, phiMin, phiMax, sigma, nEvents);
    UEvent *event = new UEvent;
    EventInitialState *iniState = new EventInitialState;
    iniTree->Branch("iniState", "EventInitialState", iniState);
    iniTree->Branch("event", "UEvent", event);

    Long64_t nentries = fTree->GetEntriesFast();
    Long64_t eventCounter = 0;
    Int_t child[2] = {0,0};

    for (Long64_t iev=0; iev<nentries; iev++)
    {
        fTree->GetEntry(iev);

        event->Clear();
        iniState->Clear();

        if (iev % 100 == 0) std::cout << "Event [" << iev << "/" << nentries << "]" << std::endl;

        if (only_not_empty && t_empty_event) continue;

        // Fill event
        event->SetEventNr(t_ev);
        event->SetB(t_impact_b);
        event->SetPhi(0.);
        event->SetNes(1);
        event->SetStepNr(0);
        event->SetStepT(t_current_t);

        // Fill particle
        for (int ipart=0; ipart<t_npart; ipart++)
        {
            event->AddParticle(ipart,t_pdgcode[ipart],0,
                    0,0,
                    0,0,child,
                    t_px[ipart],t_py[ipart],t_pz[ipart],t_p0[ipart],
                    t_x[ipart],t_y[ipart],t_z[ipart],t_t[ipart],
                    1.);
        }

        iniTree->Fill();
        eventCounter++;
    }
    header.SetNEvents(eventCounter);

    fOut->cd();
    header.Write();
    iniTree->Write();
    fOut->Close();
}
