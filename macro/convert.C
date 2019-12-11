R__LOAD_LIBRARY(libMcIniData.so)

using namespace std;

void convert(TString inputFileName="test.root", TString outputFileName="out.root")
{
  TFile *fIn = new TFile(inputFileName, "read");
  URun *run = (URun *)fIn->Get("run");
  float sqrtSnn = run->GetNNSqrtS();
  int aProj = run->GetAProj();
  TTree *tree = (TTree *)fIn->Get("events");
  EventInitialState *iniState{nullptr};
  UEvent *event{nullptr};
  tree->SetBranchAddress("iniState", &iniState);
  tree->SetBranchAddress("event", &event);

  // TH2F *h2Part = new TH2F("h2Part", "Participant position distribution;x (fm); y (fm)", 300, -15, 15, 300, -15, 15);
  // TH1F *hSpect =
  //     new TH1F("hSpect", "Projectile spectator final rapidity distribution;#it{y}; Number of particles", 500, -5, 5);

  //List of histograms and Ntuples....
  TFile *fo = new TFile(outputFileName.Data(),"recreate");
 
  TTree *mctree;

  float     d_bimp;      // impact parameter
  int       d_npart;    // number of participants
  float     d_phi2;
  float     d_phi3;
  float     d_ecc2;  // eccentricity for v2
  float     d_ecc3;  // eccentricity for v3

  static const int max_nh = 25000;

  int           d_nh;
  float         d_momx[max_nh];   //[momentum x]
  float         d_momy[max_nh];   //[momentum y]
  float         d_momz[max_nh];   //[momentum z]
  float         d_ene[max_nh];   //[energy]
  int           d_pdg[max_nh];   //[particle data group code]
  int           d_hid[max_nh];   //[history id]
  short         d_charge[max_nh];   //[electric charge]

  mctree = new TTree("mctree","Hadrons tree from MC RHIC models");

  mctree->Branch("bimp",&d_bimp,"bimp/F"); // impact parametr
  mctree->Branch("phi2",&d_phi2,"phi2/F"); // phiRP2
  mctree->Branch("phi3",&d_phi3,"phi3/F"); // phiRP3
  mctree->Branch("ecc2",&d_ecc2,"ecc2/F"); // eccentricity2
  mctree->Branch("ecc3",&d_ecc3,"ecc3/F"); // eccentricity3


  mctree->Branch("npart",&d_npart,"npart/I"); // number of participants
  mctree->Branch("nh",&d_nh,"nh/I");  // number of particles
  mctree->Branch("momx",&d_momx,"momx[nh]/F");
  mctree->Branch("momy",&d_momy,"momy[nh]/F");
  mctree->Branch("momz",&d_momz,"momz[nh]/F");
  mctree->Branch("ene",&d_ene,"ene[nh]/F");//[energy]
  mctree->Branch("hid",&d_hid,"hid[nh]/I");//[histrory id]
  mctree->Branch("pdg",&d_pdg,"pdg[nh]/I");//[particle data group code]
  mctree->Branch("charge",&d_charge,"charge[nh]/S");//[electric charge]

  Double_t aveRsqr, aveCos2, aveSin2, aveCos3, aveSin3;
  Double_t aveX, aveY, aveZ;
  Int_t    size;
  Double_t ecc2, ecc3;
  TLorentzVector vCentroid;

  int nEvents = tree->GetEntries();
  for(int iEvent = 0; iEvent < nEvents; iEvent++)
  {
    tree->GetEntry(iEvent);
    if (iEvent%100 == 0) std::cout << "Event [" << iEvent << "/" << nEvents << "]" << std::endl;

    d_bimp = event->GetB();
    d_npart= iniState->getNPart();
    d_phi2 = event->GetPhi();
    d_phi3 = 0.;

    aveX = 0.;
    aveY = 0.;
    aveZ = 0.;
    size = 0;
    vector<Nucleon> nucleons = iniState->getNucleons();
    for(auto nucleon : nucleons)
    {
      TLorentzVector position = nucleon.getPosition();
      if(nucleon.getCollisionType()==3 || nucleon.getCollisionType()==4)
      {
        // if (nucleon.getId() > 394) continue;
        aveX += position.X();
        aveY += position.Y();
        aveZ += position.Z();
        size++;
      }
    }
    aveX /= size;
    aveY /= size;
    aveZ /= size;
    vCentroid.SetXYZT(aveX,aveY,aveZ,0.);
    size = 0;
    aveRsqr = 0.;
    aveCos2 = 0.;
    aveSin2 = 0.;
    aveCos3 = 0.;
    aveSin3 = 0.;
    for(auto nucleon : nucleons)
    {
      TLorentzVector position = nucleon.getPosition();
      position -= vCentroid;
      if(nucleon.getCollisionType()==3 || nucleon.getCollisionType()==4)
      {
        // if (nucleon.getId() > 394) continue;
        size++;
        aveRsqr += position.Mag2();
        aveCos2 += position.Mag2() * TMath::Cos( 2*position.Phi() );
        aveSin2 += position.Mag2() * TMath::Sin( 2*position.Phi() );
        aveCos3 += position.Mag2() * TMath::Cos( 3*position.Phi() );
        aveSin3 += position.Mag2() * TMath::Sin( 3*position.Phi() );
      }
    }
    aveRsqr /= size;
    aveCos2 /= size;
    aveSin2 /= size;
    aveCos3 /= size;
    aveSin3 /= size;

    d_ecc2 = TMath::Sqrt(aveCos2*aveCos2 + aveSin2*aveSin2) / aveRsqr;
    d_ecc3 = TMath::Sqrt(aveCos3*aveCos3 + aveSin3*aveSin3) / aveRsqr;
    d_nh = event->GetNpa();

    for(int iPart = 0; iPart < event->GetNpa(); iPart++)
    {
      UParticle *particle = event->GetParticle(iPart);
      TLorentzVector momentum = particle->GetMomentum();

      d_momx[iPart]  = momentum.Px();
      d_momy[iPart]  = momentum.Py();
      d_momz[iPart]  = momentum.Pz();
      d_ene[iPart]   = momentum.E();
      d_pdg[iPart]   = particle->GetPdg();
      d_hid[iPart]   = particle->GetParent();
      d_charge[iPart]= 1./3.*TDatabasePDG::Instance()->GetParticle(particle->GetPdg())->Charge();
    }
    mctree->Fill();
  }
  fo->cd();
  mctree->Write();
  fo->Close();
}
