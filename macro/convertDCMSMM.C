#include "TFile.h"
#include "TMath.h"
#include "TString.h"
#include "TTree.h"

#include <fstream>
#include <iostream>

using namespace std;

bool verbose = false;

int getPdgType(int iBaryon, int iCharge, int iStrangeness = 0)
{
  int pdgType = abs(iStrangeness) * 1e7 + abs(iBaryon) * 10 + abs(iCharge) * 1e4 + 1e9;
  if(iStrangeness != 0)
    pdgType *= -abs(iStrangeness) / iStrangeness;
  return pdgType;
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
  //  ifstream inputFile;
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
  float mProton = 0.938272029;
  string line, generator = "DCM-SMM";
  const char *comment = "";
  float bWeight = 1, eBeam, bMin = 0, bMax = 20.0, phiMin = 0, phiMax = 2 * TMath::Pi(), sigma = 0, nEvents = 0;

  getline(inputFile, line);  // generator and system info
  if(line.find("DCM-SMM") > line.size())
  {
    cout << "ERROR: no mention of DCM-SMM in file header! Aborting!\n";
    return nullptr;
  }
  int sys[4];
  for(int i = 0; i < 4; i++)
  {
    line.erase(0, line.find("=") + 1);
    stringstream(line) >> sys[i];
  }
  int aProj = sys[0], zProj = sys[1], aTarg = sys[2], zTarg = sys[3];

  getline(inputFile, line);  // energy
  line.erase(0, line.find("=") + 1);
  stringstream(line) >> eBeam;

  float pProj = sqrt(0.5 * mProton * eBeam);
  float pTarg = -pProj;

  URun *header = new URun(generator.c_str(), comment, aProj, zProj, pProj, aTarg, zTarg, pTarg, bMin, bMax, bWeight,
                          phiMin, phiMax, sigma, nEvents);
  header->Print();
  for(int l = 3; l <= 20; l++)  // skip description of output structure
    getline(inputFile, line);
  return header;
}

void treatConcatenations(string &line)
{
  TString tempLine(line);
  tempLine.ReplaceAll("-", " -");
  tempLine.ReplaceAll("E -", "E-");
  line=tempLine;
}

uint fillEventInfo(string &line, UEvent *event, EventInitialState *iniState, int eventNr)
{
  treatConcatenations(line);
  if(nWordsIn(line) != 5)
  {
    cout << "Wrong number of variables in event info! Aborting!\n";
    cout << line << endl;
    assert(0);
  }
  stringstream stream(line);
  int eventId, nProduced;
  float b, bx, by;
  stream >> eventId >> nProduced >> b >> bx >> by;
  if (verbose)
  {
    cout << line << endl;
    cout << eventId << "\t" << nProduced << "\t" << b << "\t" << bx << "\t" << by << endl;
  }
  int nes = 0;
  int stepNr = 0;
  double stepT = 0;
  float phi = atan2(by, bx);
  event->SetParameters(eventNr, b, phi, nes, stepNr, stepT);
  iniState->setId(eventNr);
  return nProduced;
}

int addResidual(ifstream &inputFile, EventInitialState *iniState, int spectType)
{
  string line;
  getline(inputFile, line);
  treatConcatenations(line);
  if(nWordsIn(line) != 8)
  {
    cout << "Wrong number of variables in residual info! Aborting!\n";
    cout << line << endl;
    assert(0);
  }
  stringstream stream(line);
  int nFragments;
  float baryonNr, charge, strangeness, x = 0, y = 0, z, t, eExcitation, px, py, pz, m;
  stream >> nFragments >> baryonNr >> charge >> strangeness >> eExcitation >> px >> py >> pz;
  if(verbose)
  {
    cout << line << endl;
    cout << nFragments << "\t" << baryonNr << "\t" << charge << "\t" << strangeness << "\t" << eExcitation << "\t" << px
         << "\t" << py << "\t" << pz << endl;
  }
  x = 0;
  y = 0;
  z = eExcitation;
  if(baryonNr > 1)  // nucleus
    m = baryonNr * 0.931494;
  else if(baryonNr == 1)  // nucleon
    m = 0.938;

  TLorentzVector position(0, 0, z, t);
  TLorentzVector momentum;
  momentum.SetXYZM(px, py, pz, m);
  Nucleon nucleon;
  nucleon.setId(spectType);
  nucleon.setPdgId(getPdgType(baryonNr, charge, strangeness));
  nucleon.setPosition(position);
  nucleon.setMomentum(momentum);
  iniState->addNucleon(nucleon);

  return nFragments;
}

void addParticle(ifstream &inputFile, UEvent *event, int partIndex, int partType)
{
  string line;
  getline(inputFile, line);
  treatConcatenations(line);
  if(nWordsIn(line) != 10)
  {
    cout << "Wrong number of variables in particle info! Aborting!\n";
    cout << line << endl;
    assert(0);
  }
  stringstream stream(line);
  int charge, leptonNr, baryonNr, strangeness, pdgCode;
  int status = 0, parent = 0, parentDecay = 0, mate = 0, decay = 0, child[2] = {0, 0};
  double x = 0., y = 0., z, t = partType, weight = 1., px, py, pz, e, m;
  stream >> charge >> leptonNr >> baryonNr >> strangeness >> pdgCode >> px >> py >> pz >> e >> m;
  if(verbose)
  {
    cout << line << endl;
    cout << charge << "\t" << leptonNr << "\t" << baryonNr << "\t" << strangeness << "\t" << pdgCode << "\t" << px
         << "\t" << py << "\t" << pz << "\t" << e << "\t" << m << endl;
  }
  e = sqrt(px * px + py * py + pz * pz + m * m);
  event->AddParticle(partIndex, pdgCode, status, parent, parentDecay, mate, decay, child, px, py, pz, e, x, y, z, t,
                     weight);
}

void convertDCMSMM(TString inputFileName = "outfile.r12",
                   TString outFile = "test",
                   int nEvents = 1000,
                   int splitFactor = 1)
{
  gSystem->Load("libMcIniData.so");
  const float mProton = 0.938272029;
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
  TDatabasePDG db;
  string line;
  int nEvent=0;
  
  while(getline(inputFile, line))
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
    int nProduced = fillEventInfo(line, event, iniState, nEvent);
    int index = 0;
    for(int spectType = 1; spectType >= -1; spectType-=2) // -1 for target and 1 for projectile spectators
    {
      int nSpect = addResidual(inputFile, iniState, spectType);
      for(int iSpect = 0; iSpect < nSpect; iSpect++, index++)
      {
        addParticle(inputFile, event, iSpect, spectType);
      }
    }

    getline(inputFile, line);  // skip number of produced particles

    for(int iPart = 0; iPart < nProduced; iPart++, index++)
    {
      addParticle(inputFile, event, iPart, 0);
    }
    tree->Fill();
  }

  inputFile.close();
  header->SetNEvents(nEvent);
  header->Write();
  tree->Write();
  f->Close();
}
