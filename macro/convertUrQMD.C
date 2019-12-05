// 1. Participants - position and momentum in the last collision
// 2. Spectators - position and momentum in the first collision
// 3. Ncoll - number of inelastic collisions between nucleons and all their ancestors till 50% of the initial nucleon
// energy is dissipated - to be done. Meanwhile any inelastic scattering of nucleon (x2 if one of its ancestors is
// carrying 90% of its energy).

#include <fstream>
#include <iostream>
#include <string>

#include "TFile.h"
#include "TString.h"
#include "TTree.h"

#include "../include/EventInitialState.h"
#include "../include/UEvent.h"
#include "../include/URun.h"

R__LOAD_LIBRARY(libMcIniData.so)

using namespace std;

enum eCollisionVariables
{
  kNin = 0,
  kNout,
  kEventId,
  kB,
  kPhi,
  kCollisionVars
};

enum eParticleVariables
{
  kIndex = 0,
  kPdg,
  kPx,
  kPy,
  kPz,
  kE,
  kM,
  kX,
  kY,
  kZ,
  kT,
  kNparticleVars
};

unsigned int nWordsIn(const std::string &str)
{
  std::stringstream stream(str);
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
  else
  {
    cout << Form("--> Successfully opened file") << endl << endl;
  }
  string line;
  getline(inputFile, line);
  if(line != "# OSC1999A")  // check if it is OSCAR1999A format
  {
    cout << "ERROR: Not an OSCAR1999A file!!!\n";
    return false;
  }
  return true;
}

URun parseRunHeader(ifstream &inputFile)
{
  float mProton = 0.938272029;
  string line, generator;
  const char *comment = "";
  int aProj, zProj, aTarg, zTarg, bWeight = 0;
  string charDust;
  float eBeam, bMin = 0, bMax = 20.0, phiMin = 0, phiMax = 0, sigma = 0, nEvents = 0;

  getline(inputFile, line);       // skip "# full_event_history"
  getline(inputFile, generator);  // generator info
  generator.erase(0, 2);

  getline(inputFile, line);  // colliding system
  replace(line.begin(), line.end(), '#', ' ');
  replace(line.begin(), line.end(), '(', ' ');
  replace(line.begin(), line.end(), ')', ' ');
  replace(line.begin(), line.end(), ',', ' ');
  stringstream stream(line);
  stream >> aProj >> zProj >> charDust >> aTarg >> zTarg >> charDust >> eBeam;

  float pProj = sqrt(0.5 * mProton * eBeam);
  float pTarg = -pProj;

  URun header(generator.c_str(), comment, aProj, zProj, pProj, aTarg, zTarg, pTarg, bMin, bMax, bWeight, phiMin, phiMax,
              sigma, nEvents);
  //  cout << header->GetNNSqrtS() << endl;
  header.Print();
  return header;
}

vector<float> parseCollisionHeader(const string &line)
{
  vector<float> var(kCollisionVars);
  stringstream stream(line);
  stream >> var[kNin] >> var[kNout] >> var[kEventId] >> var[kB] >> var[kPhi];
  return var;
}

vector<float> parseParticleInfo(const string &line)
{
  vector<float> var(kNparticleVars);
  float numDust;
  stringstream stream(line);

  stream >> var[kIndex] >> var[kPdg] >> numDust >> var[kPx] >> var[kPy] >> var[kPz] >> var[kE] >> var[kM] >> var[kX] >>
      var[kY] >> var[kZ] >> var[kT];
  return var;
}

Nucleon makeNucleon(const string &line)
{
  vector<float> var = parseParticleInfo(line);
  TLorentzVector position(var[kX], var[kY], var[kZ], var[kT]);
  TLorentzVector momentum(var[kPx], var[kPy], var[kPz], var[kE]);
  Nucleon nucleon;
  nucleon.setId(var[kIndex]);
  nucleon.setPdgId(var[kPdg]);
  nucleon.setPosition(position);
  nucleon.setMomentum(momentum);
  return nucleon;
}

UParticle makeParticle(const string &line)
{
  vector<float> var = parseParticleInfo(line);
  TLorentzVector position(var[kX], var[kY], var[kZ], var[kT]);
  TLorentzVector momentum(var[kPx], var[kPy], var[kPz], var[kE]);
  int status = 0, parent = 0, parentDecay = 0, mate = 0, decay = 0, child[2] = {0, 0};
  double weight = 1.;
  UParticle particle(var[kIndex], var[kPdg], status, parent, parentDecay, mate, decay, child, momentum, position,
                     weight);
  return particle;
}

void convertUrQMD(TString inputFileName = "test.f20", TString outputFileName = "test.root", float tFinal = -1.)
{
  ifstream inputFile;
  if(!openFile(inputFile, inputFileName))
    return;
  TFile *outputFile = new TFile(outputFileName, "recreate");
  URun header = parseRunHeader(inputFile);
  TString generator;
  string line;
  header.GetGenerator(generator);
  UEvent *event = new UEvent;
  EventInitialState *iniState = new EventInitialState;
  TTree *tree = new TTree("events", generator);
  tree->Branch("iniState", "EventInitialState", iniState);
  tree->Branch("event", "UEvent", event);

  int nNucl = header.GetAProj() + header.GetATarg();
  int nColl, nPart, nIn, nOut, eventId;
  float b, phi;
  int nes = 0, stepNr = 0, stepT = 0;
  while(inputFile)  // loop over events
  {
    getline(inputFile, line);
    vector<float> eventInfo = parseCollisionHeader(line);
    eventId = eventInfo.at(kEventId);
    b = eventInfo.at(kB);
    phi = eventInfo.at(kPhi);
    if(eventId % 50 == 0)
      cout << "Event # " << eventId << "... \r" << flush;
    event->Clear();
    iniState->Clear();
    event->SetParameters(eventId, b, phi, nes, stepNr, stepT);
    iniState->setId(eventId);
    nColl = 0;
    nPart = 0;
    // Parse initial nucleon info
    for(int iNucl = 0; iNucl < nNucl; iNucl++)
    {
      getline(inputFile, line);
      Nucleon nucleon = makeNucleon(line);
      iniState->addNucleon(nucleon);
    }

    while(inputFile)  // loop over collisions in event
    {
      getline(inputFile, line);
      vector<float> collisionInfo = parseCollisionHeader(line);
      nIn = collisionInfo.at(kNin);
      nOut = collisionInfo.at(kNout);
      if(nWordsIn(line) == 2)  // beginning of final state block or end of event
        break;
      vector<vector<float>> inParticleInfo, outParticleInfo;
      for(int iIn = 0; iIn < nIn; iIn++)
      {
        getline(inputFile, line);
        inParticleInfo.push_back(parseParticleInfo(line));
      }
      for(int iOut = 0; iOut < nOut; iOut++)
      {
        getline(inputFile, line);
        outParticleInfo.push_back(parseParticleInfo(line));
      }

      if(nIn < 2)  // decay is not likely for nucleons
        continue;

      if(tFinal > 0. && inParticleInfo.at(0).at(kT) > tFinal)
        continue;

      else if(nOut == 0 || inParticleInfo.at(0).at(kIndex) != outParticleInfo.at(0).at(kIndex))  // inelastic process
      {
        if(inParticleInfo.at(0).at(kIndex) < nNucl || inParticleInfo.at(1).at(kIndex) < nNucl)
          nColl++;
        int first, second;
        for(first = 0, second = 1; first < 2; first++, second--)
        {
          if(inParticleInfo.at(first).at(kIndex) < nNucl)
          {
            TLorentzVector position(inParticleInfo.at(first).at(kX), inParticleInfo.at(first).at(kY),
                                    inParticleInfo.at(first).at(kZ), inParticleInfo.at(first).at(kT));
            TLorentzVector momentum(inParticleInfo.at(first).at(kPx), inParticleInfo.at(first).at(kPy),
                                    inParticleInfo.at(first).at(kPz), inParticleInfo.at(first).at(kE));
            ushort collisionType = inParticleInfo.at(second).at(kIndex) < nNucl ? kInelasticWithInitialNucleon :
                                                                                  kInelasticWithProducedParticle;
            Nucleon &firstNucleon = iniState->getNucleon(inParticleInfo.at(first).at(kIndex));
            firstNucleon.setPosition(position);
            firstNucleon.setMomentum(momentum);
            firstNucleon.setCollisionType(collisionType);
            firstNucleon.addCollidedNucleonIndex(inParticleInfo.at(second).at(kIndex));
            nPart++;
            for(int i = 0; i < outParticleInfo.size(); i++)
            {
              vector<float> outInfo = outParticleInfo.at(i);
              if(outInfo.at(kX) == inParticleInfo.at(first).at(kX) &&
                 outInfo.at(kY) == inParticleInfo.at(first).at(kY) &&  // one of outgoing particles 
                 outInfo.at(kE) / inParticleInfo.at(first).at(kE) > 0.9) // carries 90% of one of incoming nucleon's energy
                nColl++;
            }
          }
        }
      }

      else if(nOut == 2 && inParticleInfo.at(0).at(kIndex) == outParticleInfo.at(0).at(kIndex) &&
              inParticleInfo.at(1).at(kIndex) == outParticleInfo.at(1).at(kIndex))  // elastic scattering
      {
        int first, second;
        for(first = 0, second = 1; first < 2; first++, second--)
        {
          if(inParticleInfo.at(first).at(kIndex) < nNucl)
          {
            Nucleon &firstNucleon = iniState->getNucleon(inParticleInfo.at(first).at(kIndex));
            firstNucleon.addCollidedNucleonIndex(inParticleInfo.at(second).at(kIndex));
            if(firstNucleon.getCollisionType() == kNoCollision)
            {
              TLorentzVector position(inParticleInfo.at(first).at(kX), inParticleInfo.at(first).at(kY),
                                      inParticleInfo.at(first).at(kZ), inParticleInfo.at(first).at(kT));
              TLorentzVector momentum(inParticleInfo.at(first).at(kPx), inParticleInfo.at(first).at(kPy),
                                      inParticleInfo.at(first).at(kPz), inParticleInfo.at(first).at(kE));
              ushort collisionType = inParticleInfo.at(second).at(kIndex) < nNucl ? kElasticWithInitialNucleon :
                                                                                    kElasticWithProducedParticle;
              firstNucleon.setCollisionType(collisionType);
              firstNucleon.setPosition(position);
              firstNucleon.setMomentum(momentum);
            }
          }
        }
      }
    }

    // final state particles
    for(int i = 0; i < nIn; i++)
    {
      getline(inputFile, line);
      UParticle particle = makeParticle(line);
      event->AddParticle(particle);
    }

    getline(inputFile, line);  // end of event: "0    0"

    iniState->setNColl(nColl);
    iniState->setNPart(nPart);
    if(event->GetNpa() > nNucl)
      tree->Fill();
  }
  inputFile.close();
  header.SetNEvents(eventId);
  header.Write();
  tree->Write();
  outputFile->Close();
}
