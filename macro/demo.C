R__LOAD_LIBRARY(libMcIniData.so)

using namespace std;

void demo(TString inputFileName="test.root")
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
  TH2F *h2Part = new TH2F("h2Part", "Participant position distribution;x (fm); y (fm)", 300, -15, 15, 300, -15, 15);
  TH1F *hSpect =
      new TH1F("hSpect", "Projectile spectator final rapidity distribution;#it{y}; Number of particles", 500, -5, 5);

  int nEvents = tree->GetEntries();
  for(int iEvent = 0; iEvent < nEvents; iEvent++)
  {
    tree->GetEntry(iEvent);
    vector<Nucleon> nucleons = iniState->getNucleons();
    for(int iNucl = 0; iNucl < nucleons.size(); iNucl++)
    {
      Nucleon nucleon = nucleons.at(iNucl);
      TLorentzVector position = nucleon.getPosition();
      if(nucleon.getCollisionType()==3 || nucleon.getCollisionType()==4)
        h2Part->Fill(position.X(), position.Y());
    }
    for(int iPart = 0; iPart < event->GetNpa(); iPart++)
    {
      UParticle *particle = event->GetParticle(iPart);
      TLorentzVector momentum = particle->GetMomentum();
      if(particle->GetIndex() <= aProj)
        hSpect->Fill(momentum.Rapidity());
    }
  }
  TCanvas *c = new TCanvas("c","c",800,400);
  c->Divide(2, 1);
  c->cd(1);
  h2Part->Draw("colz");
  gPad->SetLogz();
  c->cd(2);
  hSpect->Draw();
}
