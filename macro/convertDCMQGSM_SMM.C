#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"

#include <fstream>
#include <iostream>

using namespace std;

int getPdgType (int iBaryon, int iCharge, int iStrangeness = 0)
{
  int pdgType = abs (iBaryon) * 10 + abs (iCharge) * 1e4 + 1e9;
  if (iStrangeness != 0) pdgType *= -abs (iStrangeness) / iStrangeness;
  return pdgType;
}

void convertDCMQGSM_SMM (TString inFile  = "CAS-SMM-evt.out", TString outFile = "test", int nEvents = 5000, int splitFactor = 1)
{
  gSystem->Load ("libMcIniData.so");
  const float mProton = 0.938272029;
  const float mBaryon = 0.931494;
  const char* generator = "DCMQGSM-SMM";
  const char* comment = "";
  int aProj;
  int zProj;
  double pProj;
  double T0;
  double sqrtS;
  int aTarg;
  int zTarg;
  double pTarg;
  double bMin =  0;
  double bMax =  20.0;
  int bWeight =  0;
  double phiMin = 0;
  double phiMax = 2 * TMath::Pi ();
  double sigma = 0;
  int eventsPerFile = nEvents / (float) splitFactor;

  URun *header;
  UEvent *event = new UEvent;
  EventInitialState *iniState = new EventInitialState;

  int filenum = 0;
  int eventNr, nTracks, pid, iBaryon, iCharge, iStrangeness;
  float b, bx, by, px, py, pz, e;
  float mass;
  float ExEnergy;
  int pdgType;
  int nEvent=0;
  bool skipFlag;
  double phi = 0;
  int nes = 0;
  int stepNr = 0;
  double stepT = 0;
  int status = 0;
  int parent = 0;
  int parentDecay = 0;
  int mate = 0;
  int decay = 0;
  int child[2] = {0, 0};
  double x = 0;
  double y = 0;
  double z;
  double t;
  double weight = 1;

  ifstream *InputFile = new ifstream (inFile);
  TTree *tree = NULL;
  TFile *f = NULL;
  TDatabasePDG db;
  
  *InputFile >> aProj >> zProj >> aTarg >> zTarg >> T0 >> sqrtS;
//  cout << "\t" << aProj << "\t" <<  zProj << "\t" <<  aTarg << "\t" <<  zTarg << "\t" <<  T0 << "\t" <<  sqrtS << endl;
  double E = 0.5 * sqrtS;
  pProj = sqrt(E*E-mProton*mProton);
  pTarg = - pProj;
  
  while (*InputFile >> eventNr >> b >> bx >> by)
  {
//    cout << eventNr << "\t" << b << "\t" << bx << "\t" << by << endl;
//    cout << nEvent << endl;
    if (nEvent % eventsPerFile == 0)
    {
      if (tree) tree -> Write ();
      if (f) f -> Close ();
      f = new TFile ( Form ("%s_%d.root", outFile.Data (), filenum), "recreate");
      tree = new TTree ("events", generator);
      header = new URun(generator, comment, aProj, zProj, pProj, aTarg, zTarg, pTarg, bMin, bMax, bWeight, phiMin, phiMax, sigma, eventsPerFile);
      header->Write();
      tree->Branch("event", "UEvent", event);
      tree->Branch("iniState", "EventInitialState", iniState);
      filenum++;
    }

    nEvent++;
    event -> Clear();
    iniState -> Clear();
    if ( (nEvent % 50) == 0 ) cout << "Event # " << nEvent << "... \r" << flush;

    phi = TMath::ATan2 (by, bx);

    skipFlag = false;
    event -> SetParameters (eventNr, b, phi, nes, stepNr, stepT);
    iniState -> setId(eventNr);
    int index = 0;
    
    for (int spectType = 0; spectType < 2; spectType++)
    {
      *InputFile >> iBaryon >> iCharge >> ExEnergy >> px >> py >> pz;
//      cout << iBaryon << "\t" << iCharge << "\t" << ExEnergy << "\t" << px << "\t" << py << "\t" << pz << "\t" << endl;
      z = ExEnergy;
      t = 1 - 2 * spectType; // -1 for target spectators, +1 for projectile
      pdgType = getPdgType (iBaryon, iCharge);
      TLorentzVector momentum, position;
      momentum.SetXYZM (px, py, pz, iBaryon * mBaryon);
      position.SetXYZT (0, 0, z, t);
      vector <unsigned short> collisions;
      Nucleon nucleon (spectType, pdgType, momentum, position, kInelasticWithInitialNucleon, collisions);
      iniState -> addNucleon (nucleon);
      if (t < 0. && pz > 0.) skipFlag = true;  // exclude events with positive Pz of target spectators
    }

    for (int spectType = 0; spectType < 2; spectType++)
    {
      *InputFile >> nTracks;
//      cout << nTracks << endl;

      for (int iTrack = 0; iTrack < nTracks; iTrack++, index++ )
      {
        *InputFile >> iBaryon >> iCharge >> ExEnergy >> px >> py >> pz;
//        cout << iBaryon << "\t" << iCharge << "\t" << ExEnergy << "\t" << px << "\t" << py << "\t" << pz << "\t" << endl;

        if (iBaryon > 1 && iCharge > 0)  pdgType = getPdgType (iBaryon, iCharge);
        else if (iBaryon == 1 && iCharge == 1) pdgType = 2212;
        else if (iBaryon == 1 && iCharge == 0) pdgType = 2112;
        else if (iBaryon == 0 && iCharge == 0) pdgType = 22;
        else if (iBaryon == -1 && iCharge == -1) pdgType = -2212;
        else if (iBaryon == -1 && iCharge == 0) pdgType = -2112;
        else
        {
          cout << "Undef spectator:\t" << eventNr << "\t" << iTrack << "\n" << iBaryon << "\t" << iCharge << "\t" << iStrangeness << "\t" << px << "\t" << py << "\t" << pz << "\t" << endl;
          iTrack ++;
          continue;
        }

        z = ExEnergy;
        t = 1 - 2 * spectType; // -1 for target spectators, +1 for projectile
        weight = 1;
        if (pdgType == 22) mass = 0.;
        else mass = iBaryon * 0.931494;

        e = TMath::Sqrt (px * px + py * py + pz * pz + mass * mass);

        event -> AddParticle (index, pdgType, status, parent, parentDecay, mate, decay, child, px, py, pz, e, x, y, z, t, weight);
      }
    }

    *InputFile >> nTracks;
//    cout << nTracks << endl;

    z = 0;
    t = 0;

    for (int iTrack = 0; iTrack < nTracks; iTrack++, index++ )
    {
      *InputFile >> pid >> iBaryon >> iCharge >> iStrangeness >> px >> py >> pz >> mass;
//      cout << pid << iBaryon << "\t" << iCharge << "\t" << iStrangeness << "\t"<< px << "\t" << py << "\t" << pz << "\t" << mass << endl;
      if (pid == 0)
      {
        pdgType = getPdgType (iBaryon, iCharge, iStrangeness);
      }
      else pdgType = db.ConvertIsajetToPdg(pid);
      if (pdgType == 3122 && mass > 1.120)   // lambda with sigma mass
      {
        pdgType = 3212;
        status = 1;
      }
      else status = 0;
      
      e = TMath::Sqrt(px * px + py * py + pz * pz + mass * mass);
      event -> AddParticle (index, pdgType, status, parent, parentDecay, mate, decay, child, px, py, pz, e, x, y, z, t, weight);
    }

    if (skipFlag) continue; // exclude events with positive Pz of target spectators
    tree -> Fill ();
  }

  InputFile -> close ();
  tree -> Write ();
  f -> Close ();
}
