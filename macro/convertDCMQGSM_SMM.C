#include "TFile.h"
#include "TMath.h"
#include "TString.h"
#include "TTree.h"
#include "TError.h"

#include <fstream>
#include <iostream>

using namespace std;

int verbosity = kError;

int getPdgCode (int baryonNr, int charge, int strangeness = 0)
{
  int pdgCode=0;
  if (baryonNr == 1 && charge == 1) pdgCode = 2212;
  else if (baryonNr == 1 && charge == 0) pdgCode = 2112;
  else if (baryonNr == -1 && charge == -1) pdgCode = -2212;
  else if (baryonNr == 0 && charge == 0) pdgCode = 22;
  else if (baryonNr == -1 && charge == 0) pdgCode = -2112;
  else 
  {
    pdgCode = abs (strangeness) * 1e7 + abs (baryonNr) * 10 + abs (charge) * 1e4 + 1e9;
    if (strangeness != 0) pdgCode *= -abs (strangeness) / strangeness;
  }
  if (verbosity <= kInfo) cout << "getPdgCode: " << pdgCode << endl;
  return pdgCode;
}

int getIonZ(int pdgCode) { return (pdgCode % 10000000) / 10000; }

int getIonS(int pdgCode) { return (pdgCode % (10 * 10000000)) / 10000000; }

int getIonA(int pdgCode) { return (pdgCode % 10000) / 10; }

float getMass(int pdgCode)
{
  float mBaryon=0.931494, mass;
  if (abs(pdgCode) > 1000000000)
  {
    mass = mBaryon*getIonA(pdgCode);
  }
  else 
  {
    TParticlePDG *p=TDatabasePDG::Instance()->GetParticle(pdgCode);
    if (p)
      mass = p->Mass();
    else 
    {
      cout << "Unknown PDG code: " << pdgCode << endl;
      mass = -999.;
    }
  }
  if (verbosity <= kInfo) cout << "getMass: " << mass << endl;
  return mass;
}

unsigned int nWordsIn(const std::string &line)
{
  std::stringstream stream(line);
  std::string word;
  unsigned int words = 0;
  while(stream >> word)
  {
    ++words;
  }
  return words;
}

bool openFile(ifstream &inputFile, TString inputFileName)
{
  inputFile.open(inputFileName);
  if(!inputFile)
  {
    printf("File does not exist\n");
    return false;
  }
  cout << Form("--> Successfully opened file") << endl << endl;
  return true;
}

URun *makeRunHeader(ifstream &inputFile)
{
  float mProton = TDatabasePDG::Instance()->GetParticle(2212)->Mass();
  string line, generator = "DCM-QGSM-SMM";
  const char *comment = "";
  int aProj, zProj, aTarg, zTarg;
  float eBeam, sqrtS, bWeight = 1, bMin = 0, bMax = 20.0, phiMin = 0, phiMax = 2 * TMath::Pi(), sigma = 0, nEvents = 0;

  getline(inputFile, line);  // generator and system info
  if(nWordsIn(line) != 6)
  {
    cout << "FATAL: wrong number of words in run info:" << endl << line << endl;
    return nullptr;
  }
  stringstream(line) >> aProj >> zProj >> aTarg >> zTarg >> eBeam >> sqrtS;

  if (verbosity <= kInfo) 
    cout << "\t" << aProj << "\t" << zProj << "\t" << aTarg << "\t" << zTarg << "\t" << eBeam << "\t" << sqrtS << endl;

  float pProj = sqrt(0.5 * mProton * eBeam);
  float pTarg = -pProj;

  URun *header = new URun(generator.c_str(), comment, aProj, zProj, pProj, aTarg, zTarg, pTarg, bMin, bMax, bWeight,
                          phiMin, phiMax, sigma, nEvents);
  header->Print();
  return header;
}

void treatConcatenations(string &line)
{
  TString tempLine(line);
  tempLine.ReplaceAll("-", " -");
  tempLine.ReplaceAll("E -", "E-");
  line=tempLine;
}

bool fillEventInfo(string &line, UEvent *event, EventInitialState *iniState, int eventNr)
{
  treatConcatenations(line);
  if(nWordsIn(line) != 4)
  {
    cout << "FATAL: Wrong number of variables in event info:" << endl << line << endl;
    return false;
  }
  int eventId;
  float b, bx, by;
  stringstream(line) >> eventId >> b >> bx >> by;
  if (verbosity <= kInfo)
  {
    cout << line << endl;
    cout << eventId << "\t" << b << "\t" << bx << "\t" << by << endl;
  }
  int nes = 0;
  int stepNr = 0;
  double stepT = 0;
  float phi = atan2(by, bx);
  event->SetParameters(eventNr, b, phi, nes, stepNr, stepT);
  iniState->setId(eventNr);
  return true;
}

bool addResidual(ifstream &inputFile, EventInitialState *iniState, int resType, bool &skipFlag)
{
  string line;
  getline(inputFile, line);
//  treatConcatenations(line);
  if(nWordsIn(line) != 6)
  {
    cout << "FATAL: Wrong number of variables in residual info:" << endl << line << endl;
    return false;
  }
  int baryonNr, charge, pdgCode;
  float eExcitation, x = 0, y = 0, z, t=resType, px, py, pz, m;
  stringstream(line) >> baryonNr >> charge >> eExcitation >> px >> py >> pz;
  if(verbosity <= kInfo)
  {
    cout << "addResidual:\n";
    cout << line << endl;
    cout << baryonNr << "\t" << charge << "\t" << eExcitation << "\t" << px
         << "\t" << py << "\t" << pz << endl;
  }
  z = eExcitation; // just to store it somewhere
  pdgCode = getPdgCode(baryonNr, charge);
  m = getMass(pdgCode);
  if (t < 0. && pz > 0.) skipFlag = true;  // exclude events with positive Pz of target spectators

  TLorentzVector position(x, y, z, t);
  TLorentzVector momentum;
  momentum.SetXYZM(px, py, pz, m);
  Nucleon nucleon;
  nucleon.setId(resType);
  nucleon.setPdgId(pdgCode);
  nucleon.setPosition(position);
  nucleon.setMomentum(momentum);
  iniState->addNucleon(nucleon);

  return true;
}

bool addSpectator(ifstream &inputFile, UEvent *event, int spectIndex, int spectType)
{
  string line;
  getline(inputFile, line);
  treatConcatenations(line);
  if(nWordsIn(line) != 6)
  {
    cout << "FATAL: Wrong number of variables in spectator info:"<< endl << line << endl;
    return false;
  }
  int charge, baryonNr, pdgCode=0;
  int status = 0, parent = -1, parentDecay = 0, mate = 0, decay = 0, child[2] = {0, 0};
  double eExcitation, x = 0., y = 0., z, t = spectType, weight = 1., px, py, pz, e, m;
  stringstream(line) >> baryonNr >> charge >> eExcitation >> px >> py >> pz;
  if(verbosity <= kInfo)
  {
    cout << "addSpectator:\n";
    cout << line << endl;
    cout << baryonNr << "\t" << charge << "\t" << eExcitation << "\t" << px
         << "\t" << py << "\t" << pz << endl;
  }
 
  z=eExcitation; // just to store it somewhere 
  pdgCode = getPdgCode (baryonNr, charge);
  if (pdgCode==0)
  {
    cout << "FATAL: Undefined spectator:" << endl << line << endl;
    return false;
  }
  m=getMass(pdgCode);
  if (m < 0.) 
    return false;
  e = sqrt(px * px + py * py + pz * pz + m * m);

  event->AddParticle(spectIndex, pdgCode, status, parent, parentDecay, mate, decay, child, px, py, pz, e, x, y, z, t, weight);
  return true;
}

bool addParticle(ifstream &inputFile, UEvent *event, int partIndex)
{
  string line;
  getline(inputFile, line);
  treatConcatenations(line);
  if(nWordsIn(line) != 8)
  {
    cout << "FATAL: Wrong number of variables in particle info! Aborting!\n";
    cout << line << endl;
    return false;
  }
  int charge, leptonNr, baryonNr, strangeness, pdgCode;
  int status = 0, parent = -1, parentDecay = 0, mate = 0, decay = 0, child[2] = {0, 0};
  double x = 0., y = 0., z, t = 0, weight = 1., px, py, pz, e, m;
  stringstream(line) >> pdgCode >> baryonNr >> charge >> strangeness >> px >> py >> pz >> m;
  if(verbosity <= kInfo)
  {
    cout << "addParticle:\n";
    cout << line << endl;
    cout << pdgCode << "\t" << baryonNr << "\t" << charge << "\t" << strangeness << "\t" << px
         << "\t" << py << "\t" << pz << "\t" << m << "\t" << e << endl;
  }
  e = sqrt(px * px + py * py + pz * pz + m * m);
  pdgCode = TDatabasePDG::Instance()->ConvertIsajetToPdg(pdgCode);
  if (pdgCode == 3122 && m > 1.120)
  {
    if (verbosity <= kWarning)
      cout << "WARNING: changing PDG code from 3122 (lambda) to 3212 (sigma) for a particle with mass = " << m << endl;
    pdgCode = 3212;
    status = 1;
  }
  event->AddParticle(partIndex, pdgCode, status, parent, parentDecay, mate, decay, child, px, py, pz, e, x, y, z, t,
                     weight);
  return true;
}

void convertDCMQGSM_SMM(TString inputFileName = "CAS-SMM-evt.out",
                   TString outFile = "test",
                   int nEvents = 5000,
                   int splitFactor = 1)
{
  gSystem->Load("libMcIniData.so");
  const float mProton = TDatabasePDG::Instance()->GetParticle(2212)->Mass();
  int eventsPerFile = nEvents / (float)splitFactor;

  int filenum = 0;

  ifstream inputFile;
  if(!openFile(inputFile, inputFileName))
    return;
  URun *header = makeRunHeader(inputFile);
  if(!header)
    return;
  TString generator;
  header->GetGenerator(generator);
  UEvent *event = new UEvent;
  EventInitialState *iniState = new EventInitialState;
  TTree *tree = NULL;
  TFile *f = NULL;
  string line;
  int nEvent=0;
  
  while(getline(inputFile, line) && nEvent < nEvents)
  {
    event->Clear();
    iniState->Clear();
    if(nEvent % eventsPerFile == 0)
    {
      if(f)
      {
        header->SetNEvents(nEvent);
        header->Write();
        tree->Write();
        f->Close();
        nEvent = 0;
      }
      f = new TFile(Form("%s_%d.root", outFile.Data(), filenum), "recreate");
      tree = new TTree("events", generator);
      tree->Branch("event", "UEvent", event);
      tree->Branch("iniState", "EventInitialState", iniState);
      filenum++;
    }

    if((nEvent % 50) == 0)
      cout << "Event # " << nEvent << "... \r" << flush;

    nEvent++;
    if (!fillEventInfo(line, event, iniState, nEvent))
      return;
    bool skipFlag=false;
    int index = 0;
    for(int spectType = 1; spectType >= -1; spectType-=2) // -1 for target and 1 for projectile spectators
    {
      if (!addResidual(inputFile, iniState, spectType, skipFlag)) 
        return;
    }
    for(int spectType = 1; spectType >= -1; spectType-=2) // -1 for target and 1 for projectile spectators
    {
      getline(inputFile, line);
      if (nWordsIn(line) != 1)
      {
        cout << "FATAL: Wrong number of words in number of spectators line:" << endl << line << endl;
        return;
      }
      int nSpect;
      stringstream(line) >> nSpect;
      if (verbosity <= kInfo) 
      {
        cout << "nSpect:\n";
        cout << line << endl;
        cout << nSpect << endl;
      }
      for(int iSpect = 0; iSpect < nSpect; iSpect++)
      {
        if (!addSpectator(inputFile, event, index, spectType))
          return;
        else index++;
      }
    }
    
    getline(inputFile, line);
    if (nWordsIn(line) != 1)
    {
      cout << "FATAL: Wrong number of words in number of produced line:" << endl << line << endl;
      return;
    }
    int nProduced;
    stringstream(line) >> nProduced; 
    if (verbosity <= kInfo) 
    {
      cout << "nProduced:\n";
      cout << line << endl;
      cout << nProduced << endl;
    }
    for(int iPart = 0; iPart < nProduced; iPart++)
    {
      if (!addParticle(inputFile, event, index))
        return;
      else index++;
    }
    if (!skipFlag)
      tree->Fill();
    else if (verbosity <= kError)
      cout << "ERROR: skipping event due to presence of target spectators with positive Pz!!!\n";
  }

  cout << "\rEvent # " << nEvent << endl;
  inputFile.close();
  header->SetNEvents(nEvent);
  header->Write();
  tree->Write();
  f->Close();
}
