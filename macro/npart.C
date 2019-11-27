
R__LOAD_LIBRARY(libMcIniData.so)

using namespace std;

void npart(TString inputFileName="test.root", TString outputFileName="output.root")
{
  TStopwatch timer;
  timer.Start();

  TFile *fIn = new TFile(inputFileName, "read");
  URun *run = (URun *)fIn->Get("run");
  float sqrtSnn = run->GetNNSqrtS();
  int aProj = run->GetAProj();
  TTree *tree = (TTree *)fIn->Get("events");
  EventInitialState *iniState{nullptr};
  UEvent *event{nullptr};
  tree->SetBranchAddress("iniState", &iniState);
  tree->SetBranchAddress("event", &event);
  TProfile *pEcc2vsB = new TProfile("pEcc2vsB","pEcc2vsB",200,0,20);
  TProfile *pEcc3vsB = new TProfile("pEcc3vsB","pEcc3vsB",200,0,20);
  TProfile *pNpartvsB = new TProfile("pNpartvsB","pNpartvsB",200,0,20);
  TProfile *pNcollvsB = new TProfile("pNcollvsB","pNcollvsB",200,0,20);
  TH2F *h2Part = new TH2F("h2Part", "Participant position distribution;x (fm); y (fm)", 300, -15, 15, 300, -15, 15);
  TH1F *hSpect =
      new TH1F("hSpect", "Projectile spectator final rapidity distribution;#it{y}; Number of particles", 500, -5, 5);

  int nEvents = tree->GetEntries();
  int Npart;
  double aversin2, avercos2, aver2;
  double aversin3, avercos3, aver3;
  double averX, averY;
  double posX, posY;
  double ecc2, ecc3;
  double ncount;
  double phi, rsqr;
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
        h2Part->Fill(position.X(), position.Y());
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
    if (aver2 != 0) pEcc2vsB->Fill(event->GetB(),ecc2);
    if (aver3 != 0) pEcc3vsB->Fill(event->GetB(),ecc3);
    pNpartvsB->Fill(event->GetB(),Npart);//iniState->getNPart());
    pNcollvsB->Fill(event->GetB(),iniState->getNColl());
    for(int iPart = 0; iPart < event->GetNpa(); iPart++)
    {
      UParticle *particle = event->GetParticle(iPart);
      TLorentzVector momentum = particle->GetMomentum();
      if(particle->GetIndex() <= aProj)
        hSpect->Fill(momentum.Rapidity());
    }
  }
  TFile *fo = new TFile(outputFileName.Data(),"recreate");
  fo->cd();
  pEcc2vsB->Write();
  pEcc3vsB->Write();
  pNpartvsB->Write();
  pNcollvsB->Write();

  timer.Stop();
  timer.Print();
}
