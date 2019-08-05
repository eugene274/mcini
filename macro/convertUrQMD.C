#include "TFile.h"
#include "TMath.h"
#include "TString.h"
#include "TTree.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <fstream>
#include <iostream>
#include <locale>
#include <map>
#include <string>

R__LOAD_LIBRARY(libMcIniData.so)

using namespace std;

void convertUrQMD(TString inputFileName = "test.f20", TString outputFileName = "test.root")
{
  bool parse_verbosity{true};
  double mProton = 0.938272029;

  ifstream inputFile;
  string generator;
  const char *comment = "";
  int aProj, zProj, aTarg, zTarg;
  double pProj, pTarg, sqrtSnn;
  double phi, b, numDust, px, py, pz, e, m, x, y, z, t;
  string charDust;
  int id, id2, pdg, iEvent, nColl, nPart;
  int nes = 0;
  int stepNr = 0;
  double stepT = 0;
  double bMin = 0;
  double bMax = 20.0;
  int bWeight = 0;
  double phiMin = 0;
  double phiMax = 0;
  double sigma = 0;

  inputFile.open(inputFileName);
  if(!inputFile)
  {
    printf("File does not exist\n");
    return;
  }
  else
  {
    cout << Form("--> Successfully opened file") << endl << endl;
  }
  string linestr;
  TFile *outputFile = new TFile(outputFileName, "recreate");
  UEvent *event = new UEvent;
  EventInitialState *iniState = new EventInitialState;
  TTree *tree = new TTree("events", generator.c_str());
  tree->Branch("iniState", "EventInitialState", iniState);
  tree->Branch("event", "UEvent", event);

  getline(inputFile, linestr);
  if(linestr != "# OSC1999A")  // check if it is OSCAR1999A format
  {
    cout << "ERROR: Not an OSCAR1999A file!!!\n";
    return;
  }
  getline(inputFile, linestr);    // skip "# full_event_history"
  getline(inputFile, generator);  // generator info
  generator.erase(0, 2);

  getline(inputFile, linestr);  // colliding system
  replace(linestr.begin(), linestr.end(), '#', ' ');
  replace(linestr.begin(), linestr.end(), '(', ' ');
  replace(linestr.begin(), linestr.end(), ')', ' ');
  replace(linestr.begin(), linestr.end(), ',', ' ');
  istringstream iss(linestr);
  iss >> aProj >> zProj >> charDust >> aTarg >> zTarg >> charDust >> sqrtSnn;
  //  cout << linestr << endl;
  //  cout << zProj << "\t" << aProj << "\t" << charDust << "\t" << zTarg << "\t" << aTarg << "\t" << charDust << "\t"
  //       << sqrtSnn << "\t" << endl;

  int nNucl = aProj + aTarg;
  pProj = sqrt(0.25 * sqrtSnn * sqrtSnn - mProton * mProton);
  pTarg = -pProj;

  URun *header = new URun(generator.c_str(), comment, aProj, zProj, pProj, aTarg, zTarg, pTarg, bMin, bMax, bWeight,
                          phiMin, phiMax, sigma, iEvent);
  header->Dump();
  while(inputFile)
  {
    getline(inputFile, linestr);
    istringstream iss1(linestr);
    int nIn = 0, nOut = 0;
    iss1 >> nIn >> nOut >> iEvent >> b >> phi;
//    cout << linestr << endl;
//    cout << "\t" << nIn << "\t" << nOut << "\t" << iEvent << "\t" << b << "\t" << phi << endl;

    // Find new event header
    //    if(nIn == 0 && nOut == nNucl)
    //    {
    if(iEvent % 50 == 0)
      cout << "Event # " << iEvent << "... \r" << flush;
    event->Clear();
    iniState->Clear();
    event->SetParameters(iEvent, b, phi, nes, stepNr, stepT);
    iniState->setId(iEvent);
    nColl = 0;
    nPart = 0;

    // Parse initial nucleon info
    for(int iNucl = 0; iNucl < nNucl; iNucl++)
    {
      getline(inputFile, linestr);
      istringstream iss2(linestr);
      iss2 >> id >> pdg >> numDust >> px >> py >> pz >> e >> m >> x >> y >> z >> t;
      //        cout << linestr << endl;
      //        cout << "\t" << id << "\t" << pdg << "\t" << numDust << "\t" << px << "\t" << py << "\t" << pz << "\t"
      //        << e << "\t"
      //             << m << "\t" << x << "\t" << y << "\t" << z << "\t" << t << "\t" << endl;
      TLorentzVector momentum(px, py, pz, e);
      TLorentzVector position(x, y, z, t);
      Nucleon nucleon;
      nucleon.setId(iNucl);
      nucleon.setPdgId(pdg);
      nucleon.setMomentum(momentum);
      nucleon.setPosition(position);
      iniState->addNucleon(nucleon);
    }

    // Find collision header in the current event for 2 -> many scatterings
    while(inputFile)
    {
      int processType = -1;
      getline(inputFile, linestr);
      istringstream iss2(linestr);
      iss2 >> nIn >> nOut >> processType;
//      cout << linestr << endl;
//      cout << nIn << "\t" << nOut << "\t" << processType << endl;
      if(processType == -1)  // end of initial state block
        break;
      getline(inputFile, linestr);
      //cout << linestr << endl;
      if(nIn == 2)
      {
        istringstream iss3(linestr);
        iss3 >> id;
        getline(inputFile, linestr);
        //cout << linestr << endl;
        istringstream iss4(linestr);
        iss4 >> id2;      
        // << id << "\t" << id2 << endl;
        if(id <= nNucl && id2 <= nNucl)  // scattering between primary nucleons
        {
  //        cout << linestr << endl;
  //        cout << "\t" << id << "\t" << pdg << "\t" << numDust << "\t" << px << "\t" << py << "\t" << pz << "\t" << e
  //             << "\t" << m << "\t" << x << "\t" << y << "\t" << z << "\t" << t << "\t" << endl;

          nColl++;
          // add collided nucleon to index array
          TLorentzVector position;
          iniState->getNucleon(id).addCollidedNucleonIndex(id2);
          iniState->getNucleon(id2).addCollidedNucleonIndex(id);
          // update information on coordinates and momenta
          if(iniState->getNucleon(id).isSpect())
          {
            iss3 >> id >> pdg >> numDust >> px >> py >> pz >> e >> m >> x >> y >> z >> t;
            position.SetXYZT(x, y, z, t);
            iniState->getNucleon(id).setPosition(position);
            nPart++;
          }
          if(iniState->getNucleon(id2).isSpect())
          {
            iss4 >> id >> pdg >> numDust >> px >> py >> pz >> e >> m >> x >> y >> z >> t;
            position.SetXYZT(x, y, z, t);
            iniState->getNucleon(id).setPosition(position);
            nPart++;
          }
        }
      }
      // Skip the lines corresponding to collision products
      for(int i = 0; i < nOut; i++)
      {
        getline(inputFile, linestr);
        //        cout << linestr << endl;
      }
    }
    iniState->setNColl(nColl);
    iniState->setNPart(nPart);

    // Identify final-state particles and store them
//    cout << linestr << endl;
//    cout << nIn << endl;
    if(nIn > 0 && nOut == 0)
    {
      for(int i = 0; i < nIn; i++)
      {
        int status = 0;
        int parent = 0;
        int parentDecay = 0;
        int mate = 0;
        int decay = 0;
        int child[2] = {0, 0};
        double weight = 1.;

        std::getline(inputFile, linestr);
        istringstream iss5(linestr);
        iss5 >> id >> pdg >> numDust >> px >> py >> pz >> e >> m >> x >> y >> z >> t;
        //        cout << linestr << endl;
        //        cout << "\t" << id << "\t" << pdg << "\t" << numDust << "\t" << px << "\t" << py << "\t" << pz << "\t"
        //        << e
        //             << "\t" << m << "\t" << x << "\t" << y << "\t" << z << "\t" << t << "\t" << endl;
        TLorentzVector momentum(px, py, pz, e);
        TLorentzVector position(x, y, z, t);
        event->AddParticle(id, pdg, status, parent, parentDecay, mate, decay, child, momentum, position, weight);
      }
    }
    if (event->GetNpa() > nNucl)
      tree->Fill();
    getline(inputFile, linestr);
  }

  inputFile.close();
  header->SetNEvents(iEvent);
  header->Write();
  tree->Write();
  outputFile->Close();
}
