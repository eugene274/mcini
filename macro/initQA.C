
R__LOAD_LIBRARY(libMcIniData.so)

using namespace std;

void initQA(TString inputFileName="test.root", TString outputFileName="output.root")
{
  TStopwatch timer;
  timer.Start();

  const int nCollTypes = 8;
  // Nucleon scattering types here:
  // 0 -- No collisions at all (T0)
  // 1 -- Elastic collisions with the initial nucleons   (TA)
  // 2 -- Elastic collisions with produced particles     (TB)
  // 3 -- Inelastic collisions with the initial nucleons (TC)
  // 4 -- Inelastic collisions with produced particles   (TD)
  // 5 -- All inelastic collisions                       (TC+TD)
  // 6 -- All occured collisions                         (TA+TB+TC+TD)
  // 7 -- All types of collisions                        (T0+TA+TB+TC+TD)

  TFile *fIn = new TFile(inputFileName, "read");
  URun *run = (URun *)fIn->Get("run");
  float sqrtSnn = run->GetNNSqrtS();
  int aProj = run->GetAProj();
  TTree *tree = (TTree *)fIn->Get("events");
  EventInitialState *iniState{nullptr};
  UEvent *event{nullptr};
  tree->SetBranchAddress("iniState", &iniState);
  tree->SetBranchAddress("event", &event);
  TH1F     *hBimp = new TH1F("hBimp","hBimp",200,0,20);

  TProfile *pEcc2vsB[nCollTypes];
  TProfile *pEcc3vsB[nCollTypes];
  TProfile *pEcc2RPvsB[nCollTypes];
  TProfile *pEcc3RPvsB[nCollTypes];
  TProfile *pNscvsB[nCollTypes];
  TProfile *pNuclTvsB[nCollTypes];

  TH2D     *hNuclXY[nCollTypes];
  TH2D     *hNuclEtaPt[nCollTypes];
  TH2D     *hNuclYPt[nCollTypes];
  TH1D     *hNuclT[nCollTypes];

  TProfile *pNscCorr[nCollTypes][nCollTypes];
  TH2I     *hNscCorr[nCollTypes][nCollTypes];

  for (int i=0; i<nCollTypes; i++)
  {
    pEcc2vsB[i] = new TProfile(Form("pEcc2vsB%i",i),Form("pEcc2vsB%i",i),200,0,20);
    pEcc3vsB[i] = new TProfile(Form("pEcc3vsB%i",i),Form("pEcc3vsB%i",i),200,0,20);
    pEcc2RPvsB[i] = new TProfile(Form("pEcc2RPvsB%i",i),Form("pEcc2RPvsB%i",i),200,0,20);
    pEcc3RPvsB[i] = new TProfile(Form("pEcc3RPvsB%i",i),Form("pEcc3RPvsB%i",i),200,0,20);
    pNscvsB[i]  = new TProfile(Form("pNscvsB%i",i), Form("pNscvsB%i",i), 200,0,20);
    pNuclTvsB[i] = new TProfile(Form("pNuclTvsB%i",i), Form("pNuclTvsB%i",i), 200,0,20);
    hNuclXY[i]  = new TH2D(Form("hNuclXY%i",i),Form("hNuclXY%i",i),300,-15,15,300,-15,15);
    hNuclEtaPt[i] = new TH2D(Form("hNuclEtaPt%i",i),Form("hNuclEtaPt%i",i),400,-10.,10.,500,0.,5.);
    hNuclYPt[i] = new TH2D(Form("hNuclYPt%i",i),Form("hNuclYPt%i",i),400,-10.,10.,500,0.,5.);
    hNuclT[i] = new TH1D(Form("hNuclT%i",i),Form("hNuclT%i",i),2000,0.,200.);
    for (int j=0; j<nCollTypes; j++)
    {
      pNscCorr[i][j] = new TProfile(Form("pNscCorr_%i_%i",i,j), Form("pNscCorr_%i_%i",i,j), 394,0,394);
      hNscCorr[i][j] = new TH2I    (Form("hNscCorr_%i_%i",i,j), Form("hNscCorr_%i_%i",i,j), 394,0,394, 394,0,394);
    }
  }

  int nEvents = tree->GetEntries();
  int Nscatt[nCollTypes];
  double aversin2, avercos2, aver2;
  double aversin3, avercos3, aver3;
  double averX, averY;
  double posX, posY;
  double ecc2, ecc3;
  double ecc2RP, ecc3RP;
  double ncount;
  double phi, rsqr;
  double Ecc2[nCollTypes], Ecc3[nCollTypes];
  for(int iEvent = 0; iEvent < nEvents; iEvent++)
  {
    if (iEvent%1000 == 0) std::cout << "Event [" << iEvent << "/" << nEvents << "]" << std::endl;
    tree->GetEntry(iEvent);
    hBimp->Fill(event->GetB());
    for (int i=0; i<nCollTypes; i++)
    {
      Nscatt[i] = 0;
      Ecc2[i] = 0;
      Ecc3[i] = 0;
    }
    vector<Nucleon> nucleons = iniState->getNucleons();
   
    for (int i=0; i<nCollTypes; i++)
    {
      aversin2 = 0.; avercos2 = 0.; aver2 = 0.;
      aversin3 = 0.; avercos3 = 0.; aver3 = 0.;
      averX = 0.; averY = 0.;
      ncount = 0;
      for(auto nucleon : nucleons)
      {
        TLorentzVector position = nucleon.getPosition();
        TLorentzVector momentum = nucleon.getMomentum();
        //if (position.T() > 2.830) continue;
        if((i < 5 && nucleon.getCollisionType()==i) ||
          ((i >= 5 && i < 6) && (nucleon.getCollisionType()==3 || nucleon.getCollisionType()==4)) ||
          (i == 6 && nucleon.getCollisionType()!= 0) ||
          (i == 7))
        {
          ncount++;
          averX += position.X();
          averY += position.Y();
          hNuclXY[i]->Fill(position.X(),position.Y());
          hNuclEtaPt[i]->Fill(momentum.Eta(),momentum.Pt());
          hNuclYPt[i]->Fill(momentum.Rapidity(),momentum.Pt());
          hNuclT[i]->Fill(position.T());
          pNuclTvsB[i]->Fill(event->GetB(),position.T());
        }
      }
      if (ncount == 0) continue;
      averX /= ncount;
      averY /= ncount;

      ncount = 0;
      for(auto nucleon : nucleons)
      {
        TLorentzVector position = nucleon.getPosition();
        //if (position.T() > 2.830) continue;
        if((i < 5 && nucleon.getCollisionType()==i) ||
          ((i >= 5 && i < 6) && (nucleon.getCollisionType()==3 || nucleon.getCollisionType()==4)) ||
          (i == 6 && nucleon.getCollisionType()!= 0) ||
          (i == 7))
        {
          posX = position.X() - averX;
          posY = position.Y() - averY;
          rsqr = posX*posX + posY*posY;
          phi = TMath::ATan2(posY,posX);
          
          Nscatt[i]++;
        
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
      if (aver2 != 0) ecc2RP = avercos2 / aver2;
      if (aver3 != 0) ecc3RP = avercos3 / aver3;
      if (iEvent%1000 == 0) std::cout << "\tNpart(std) = " << iniState->getNPart()
        << " Nscattered[" << i << "] = " << Nscatt[i] << std::endl;
      if (aver2 != 0) pEcc2vsB[i]->Fill(event->GetB(),ecc2);
      if (aver3 != 0) pEcc3vsB[i]->Fill(event->GetB(),ecc3);
      if (aver2 != 0) pEcc2RPvsB[i]->Fill(event->GetB(),ecc2RP);
      if (aver3 != 0) pEcc3RPvsB[i]->Fill(event->GetB(),ecc3RP);
      pNscvsB[i]->Fill(event->GetB(),Nscatt[i]);
    }
    for (int i=0; i<nCollTypes; i++)
    {
      for (int j=0; j<nCollTypes; j++)
      {
        pNscCorr[i][j]->Fill(Nscatt[i],Nscatt[j]);
        hNscCorr[i][j]->Fill(Nscatt[i],Nscatt[j]);
      }
    }
  }

  TFile *fo = new TFile(outputFileName.Data(),"recreate");
  fo->cd();
  hBimp->Write();
  fo->mkdir("ValvsB");
  fo->mkdir("Nscattered_Correlations");
  fo->mkdir("Nscattered_Correlations/Profiles");
  fo->mkdir("Nscattered_Correlations/Histograms");
  fo->cd("ValvsB");
  for (int i=0; i<nCollTypes; i++)
  {
    pEcc2vsB[i]->Write();
    pEcc3vsB[i]->Write();
    pEcc2RPvsB[i]->Write();
    pEcc3RPvsB[i]->Write();
    pNscvsB[i]->Write();
    pNuclTvsB[i]->Write();
    hNuclXY[i]->Write();
    hNuclEtaPt[i]->Write();
    hNuclYPt[i]->Write();
    hNuclT[i]->Write();
  }
  fo->cd("Nscattered_Correlations/Profiles");
  for (int i=0; i<nCollTypes; i++)
  {
    for (int j=0; j<nCollTypes; j++)
    {
      pNscCorr[i][j]->Write();
    }
  }
  fo->cd("Nscattered_Correlations/Histograms");
  for (int i=0; i<nCollTypes; i++)
  {
    for (int j=0; j<nCollTypes; j++)
    {
      hNscCorr[i][j]->Write();
    }
  }

  timer.Stop();
  timer.Print();
}
