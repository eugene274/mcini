
R__LOAD_LIBRARY(libMcIniData.so)

using namespace std;

void eccCent(TString inputFileName="test.root", TString outputFileName="output.root", TString centFileName="HistoCutResults.root")
{
  TStopwatch timer;
  timer.Start();
  
  const Int_t CentralityClasses = 10;

  TFile *fIn = new TFile(inputFileName, "read");
  URun *run = (URun *)fIn->Get("run");
  float sqrtSnn = run->GetNNSqrtS();
  int aProj = run->GetAProj();
  TTree *tree = (TTree *)fIn->Get("events");
  EventInitialState *iniState{nullptr};
  UEvent *event{nullptr};
  tree->SetBranchAddress("iniState", &iniState);
  tree->SetBranchAddress("event", &event);

  TProfile *pEcc2vsCent = new TProfile("pEcc2vsCent","pEcc2vsCent",10,0,100);
  TProfile *pEcc3vsCent = new TProfile("pEcc3vsCent","pEcc3vsCent",10,0,100);
  TH2F *hEcc2vsCent = new TH2F("hEcc2vsCent","hEcc2vsCent",10,0,100,100,0.,1.);
  TH2F *hEcc3vsCent = new TH2F("hEcc3vsCent","hEcc3vsCent",10,0,100,100,0.,1.);

  TFile *centFile = new TFile(centFileName.Data(),"READ");
  TTree *Borders = (TTree*)centFile->Get("Borders");
  Int_t entries, MinBorder, MaxBorder;
  Borders->SetBranchAddress("MinBorder", &MinBorder);
  Borders->SetBranchAddress("MaxBorder", &MaxBorder);
  entries = Borders->GetEntries();
  map<Int_t,pair<Int_t,Int_t>> mBorders;
  for (int i=0; i<entries;i++)
  {
    Borders->GetEntry(i);
    mBorders[i] = {MinBorder, MaxBorder};
  }
  centFile->Close();

  int nEvents = tree->GetEntries();
  int Npart;
  double aversin2, avercos2, aver2;
  double aversin3, avercos3, aver3;
  double averX, averY;
  double posX, posY;
  double ecc2, ecc3;
  double ncount;
  double phi, rsqr;
  double pt, eta, p, charge;
  long refMult;
  int cent;
  for(int iEvent = 0; iEvent < nEvents; iEvent++)
  {
    if (iEvent%1000 == 0) std::cout << "Event [" << iEvent << "/" << nEvents << "]" << std::endl;
    tree->GetEntry(iEvent);
    Npart = 0;
    vector<Nucleon> nucleons = iniState->getNucleons();
    aversin2 = 0.; avercos2 = 0.; aver2 = 0.;
    aversin3 = 0.; avercos3 = 0.; aver3 = 0.;
    averX = 0.; averY = 0.;
    ncount = 0;
    for(auto nucleon : nucleons)
    {
      TLorentzVector position = nucleon.getPosition();
      if(nucleon.getCollisionType()==3 || nucleon.getCollisionType()==4)
      {
        ncount++;
        averX += position.X();
        averY += position.Y();
      }
    }
    if (ncount == 0) continue;
    averX /= ncount;
    averY /= ncount;

    ncount = 0;
    for(auto nucleon : nucleons)
    {
      TLorentzVector position = nucleon.getPosition();
      if(nucleon.getCollisionType()==3 || nucleon.getCollisionType()==4)
      {
        posX = position.X() - averX;
        posY = position.Y() - averY;
        rsqr = posX*posX + posY*posY;
        phi = TMath::ATan2(posY,posX);
        
        Npart++;
        
        aversin2 += rsqr*TMath::Sin(2.*phi);
        avercos2 += rsqr*TMath::Cos(2.*phi);
        aver2    += rsqr;

        aversin3 += TMath::Power(rsqr,2./2.)*TMath::Sin(3.*phi);
        avercos3 += TMath::Power(rsqr,2./2.)*TMath::Cos(3.*phi);
        aver3    += TMath::Power(rsqr,2./2.);
        
        ncount++;
      }
    }
    aversin2 /= ncount;
    avercos2 /= ncount;
    aver2    /= ncount;
    aversin3 /= ncount;
    avercos3 /= ncount;
    aver3    /= ncount;
    if (aver2 != 0) ecc2 = TMath::Sqrt(avercos2*avercos2 + aversin2*aversin2)/ aver2;
    if (aver3 != 0) ecc3 = TMath::Sqrt(avercos3*avercos3 + aversin3*aversin3)/ aver3;
    if (iEvent%1000 == 0) std::cout << "\tNpart(std) = " << iniState->getNPart()
      << " Npart(custom) = " << Npart++ << std::endl;
 
    refMult = 0;
    for (int iPart=0; iPart<event->GetNpa(); iPart++)
    {
      UParticle *particle = event->GetParticle(iPart);
      charge = 3*TDatabasePDG::Instance()->GetParticle(particle->GetPdg())->Charge();
      if (charge == 0) continue;
      pt = particle->GetMomentum().Pt();
      eta= particle->GetMomentum().Eta();
      if (pt < 0.15) continue;
      if (TMath::Abs(eta) > 0.5) continue;

      refMult++;
    }

    cent = -1;
    for (auto const& it : mBorders)
    {
      if (refMult >= it.second.first && refMult < it.second.second) cent = it.first;
    }
    if (cent == -1) continue;

    if (aver2 != 0)
    {
      pEcc2vsCent->Fill(10.*cent,ecc2);
      hEcc2vsCent->Fill(10.*cent,ecc2);
    }
    if (aver3 != 0)
    {
      pEcc3vsCent->Fill(10.*cent,ecc3);
      hEcc3vsCent->Fill(10.*cent,ecc3);
    }
  }

  TFile *fo = new TFile(outputFileName.Data(),"recreate");
  fo->cd();
  pEcc2vsCent->Write();
  pEcc3vsCent->Write();
  hEcc2vsCent->Write();
  hEcc3vsCent->Write();

  timer.Stop();
  timer.Print();
}
