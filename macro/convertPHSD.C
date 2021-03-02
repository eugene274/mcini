#include "TFile.h"
#include "TMath.h"
#include "TString.h"
#include "TTree.h"

#include <fstream>
#include <iostream>

using namespace std;

bool verbose = false;

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
  cout << inputFileName << endl;
  inputFile.open(inputFileName);
  if(!inputFile)
  {
    cout << "File does not exist!";
    return false;
  }
  cout << "--> Successfully opened file!" << endl;
  return true;
}

URun *makeRunHeader(ifstream &inputFile)
{
  const float mProton = TDatabasePDG::Instance()->GetParticle(2212)->Mass();
  string generator = "PHSD";
  const char *comment = "";
  float value, eKin, bWeight = 1., bMin, bMax, phiMin = 0., phiMax = 0., sigma = 0.;
  int aProj, aTarg, zProj, zTarg, eventsPerRun, nRuns;
  string line, comma, valueName;

  while(getline(inputFile, line))
  {
    stringstream ss(line);
    ss >> value >> comma >> valueName;
    if(valueName == "MASSTA:")
      aTarg = int(value);
    else if(valueName == "MSTAPR:")
      zTarg = int(value);
    else if(valueName == "MASSPR:")
      aProj = int(value);
    else if(valueName == "MSPRPR:")
      zProj = int(value);
    else if(valueName == "ELAB:")
      eKin = value;
    else if(valueName == "BMIN:")
      bMin = value;
    else if(valueName == "BMAX:")
      bMax = value;
    else if(valueName == "NUM:")
      eventsPerRun = value;
    else if(valueName == "ISUBS:")
      nRuns = value;
  }

  float pProj = sqrt(0.5 * mProton * eKin);
  float pTarg = -pProj;
  int nEvents = eventsPerRun * nRuns;

  URun *header = new URun(generator.c_str(), comment, aProj, zProj, pProj, aTarg, zTarg, pTarg, bMin, bMax, bWeight,
                          phiMin, phiMax, sigma, nEvents);

  if(verbose)
  {
    cout << line << endl;
  }
  header->Print();
  return header;
}

uint fillEventInfo(string &line, UEvent *event, int eventNr)
{
  if(nWordsIn(line) != 5)
  {
    cout << "Wrong number of variables in event info! Aborting!\n";
    cout << line << endl;
    assert(0);
  }
  stringstream stream(line);
  int eventId, runId, nProduced;
  float b, bWeight;
  stream >> nProduced >> eventId >> runId >> b >> bWeight;
  if(verbose)
  {
    cout << line << endl;
    cout << nProduced << "\t" << eventId << "\t" << runId << "\t" << b << "\t" << bWeight << endl;
  }
  int nes = 0;
  int stepNr = 0;
  double stepT = 0;
  float phi = 0.;
  event->SetParameters(eventNr, b, phi, nes, stepNr, stepT);
  return nProduced;
}

void fillInitialStateInfo(string &line, UEvent *event)
{
  if(nWordsIn(line) != 9)
  {
    cout << "Wrong number of variables in initial state info! Aborting!\n";
    cout << line << endl;
    assert(0);
  }
  stringstream stream(line);
  int nPart;
  float psi[4], ecc[4];
  stream >> nPart;
  for(int i = 0; i < 4; i++)
  {
    stream >> psi[i] >> ecc[i];
  }
  if(verbose)
  {
    cout << line << endl;
    cout << nPart;
    for(int i = 0; i < 4; i++)
    {
      cout << "\t" << psi[i] << "\t" << ecc[i];
    }
    cout << endl;
  }
  event->SetNes(nPart);
}

void addParticle(ifstream &inputFile, UEvent *event, int partIndex)
{
  string line;
  getline(inputFile, line);
  int nWords = nWordsIn(line);
  if(nWords != 7 && nWords != 8 && nWords != 9)
  {
    cout << "Wrong number of variables in particle info! Aborting!\n";
    cout << line << endl;
    assert(0);
  }
  stringstream stream(line);
  int pdgCode, charge;
  int status = 0, parent, parentDecay = 0, mate = -2, decay = 0, child[2] = {0, 0};
  double x = 0., y = 0., z = 0., t = 0., weight = 1., px, py, pz, e;
  stream >> pdgCode >> charge >> px >> py >> pz >> e >> parent >> mate;
  if(verbose)
  {
    cout << line << endl;
    cout << pdgCode << "\t" << charge << "\t" << px << "\t" << py << "\t" << pz << "\t" << e << "\t" << parent << "\t"
         << mate << endl;
  }
  event->AddParticle(partIndex, pdgCode, status, parent, parentDecay, mate, decay, child, px, py, pz, e, x, y, z, t,
                     weight);
}

void convertPHSD(
    TString inputFileName = "/home/ogolosov/desktop/analysis/mc/dat/input02632",
    TString datFileName = "/home/ogolosov/desktop/analysis/mc/dat/phsd40csr_qgp_w.auau.10gev.mbias.02632.dat",
    TString outFile = "test",
    int splitFactor = 1)
{
  ifstream datFile, inputFile;
  if(!openFile(inputFile, inputFileName))
    return;
  if(!openFile(datFile, datFileName))
    return;
  URun *header = makeRunHeader(inputFile);
  if(!header)
    return;
  inputFile.close();
  int eventsPerFile = header->GetNEvents() / (float)splitFactor;
  int filenum = 0;
  int nEvent = 0;
  TString generator;
  header->GetGenerator(generator);
  UEvent *event = new UEvent;
  TTree *tree = NULL;
  TFile *f = NULL;
  string line;

  while(getline(datFile, line))
  {
    event->Clear();
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
      filenum++;
    }

    if((nEvent % 50) == 0)
      cout << "Event # " << nEvent << "... \r" << flush;

    nEvent++;
    int nProduced = fillEventInfo(line, event, nEvent);

    int index = 0;

    getline(datFile, line);
    fillInitialStateInfo(line, event);

    for(int iPart = 0; iPart < nProduced; iPart++, index++)
    {
      addParticle(datFile, event, iPart);
    }
    tree->Fill();
  }

  datFile.close();
  header->SetNEvents(nEvent);
  header->Write();
  tree->Write();
  f->Close();
}
