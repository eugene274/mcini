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

void convertUrQMD(TString inputFileName = "test.f20", TString outputFileName = "test.root")
{
  bool parse_verbosity{true};
  double mProton = 0.938272029;

  ifstream inputFile;
  string generator;
  const char *comment = "";
  int aProj, zProj, aTarg, zTarg;
  double pProj, pTarg, eBeam;
  double phi, b, numDust, px, py, pz, e, m, x, y, z, t;
  string charDust;
  int id, id2, id3, id4, pdg, iEvent, nColl, nPart;
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
  iss >> aProj >> zProj >> charDust >> aTarg >> zTarg >> charDust >> eBeam;
  //  cout << linestr << endl;
  //  cout << zProj << "\t" << aProj << "\t" << charDust << "\t" << zTarg << "\t" << aTarg << "\t" << charDust << "\t"
  //       << eBeam << "\t" << endl;

  int nNucl = aProj + aTarg;
  pProj = sqrt(0.5 * mProton * eBeam);
  pTarg = -pProj;

  URun *header = new URun(generator.c_str(), comment, aProj, zProj, pProj, aTarg, zTarg, pTarg, bMin, bMax, bWeight,
                          phiMin, phiMax, sigma, iEvent);
  //  cout << header->GetNNSqrtS() << endl;
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
      nucleon.setId(iNucl+1);
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
      // cout << linestr << endl;
      // cout << nIn << "\t" << nOut << "\t" << processType << endl;
      if(processType == -1)  // end of initial state block
        break;
      // cout << linestr << endl;
      if(nIn == 2 && nOut >= 2)
      {
        getline(inputFile, linestr);
        istringstream iss3(linestr);
        iss3 >> id;
        getline(inputFile, linestr);
        istringstream iss4(linestr);
        iss4 >> id2;
        getline(inputFile, linestr);
        istringstream iss5(linestr);
        iss5 >> id3;
        getline(inputFile, linestr);
        istringstream iss6(linestr);
        iss6 >> id4;

        // cout << id << "\t" << id2 << "\t" << id3 << "\t" << id4 << endl;
        TLorentzVector position;
        TLorentzVector momentum;
        if(id <= nNucl && id2 <= nNucl)  // scattering between primary nucleons
        {
          nColl++;
          // add collided nucleon to index array
          // update information on coordinates and momenta
          if(iniState->getNucleon(id).isSpect())
            nPart++;
          if(iniState->getNucleon(id2).isSpect())
            nPart++;

          iniState->getNucleon(id2).addCollidedNucleonIndex(id);
          iniState->getNucleon(id).addCollidedNucleonIndex(id2);

          iss3 >> pdg >> numDust >> px >> py >> pz >> e >> m >> x >> y >> z >> t;
          position.SetXYZT(x, y, z, t);
          momentum.SetPxPyPzE(px, py, pz, e);
          iniState->getNucleon(id).setPosition(position);
          iniState->getNucleon(id).setMomentum(momentum);
          /*          cout << linestr << endl;
                    cout << "\t" << id << "\t" << pdg << "\t" << numDust << "\t" << px << "\t" << py << "\t" << pz <<
             "\t" << e
                         << "\t" << m << "\t" << x << "\t" << y << "\t" << z << "\t" << t << "\t" << endl;*/

          iss4 >> pdg >> numDust >> px >> py >> pz >> e >> m >> x >> y >> z >> t;
          position.SetXYZT(x, y, z, t);
          momentum.SetPxPyPzE(px, py, pz, e);
          iniState->getNucleon(id2).setPosition(position);
          iniState->getNucleon(id2).setMomentum(momentum);
          //          cout << linestr << endl;
          //          cout << "\t" << id2 << "\t" << pdg << "\t" << numDust << "\t" << px << "\t" << py << "\t" << pz <<
          //          "\t" << e
          //               << "\t" << m << "\t" << x << "\t" << y << "\t" << z << "\t" << t << "\t" << endl;
          if(id == id3)
          {
            iss5 >> pdg >> numDust >> px >> py >> pz >> e >> m >> x >> y >> z >> t;
            momentum.SetPxPyPzE(px, py, pz, e);
            iniState->getNucleon(id).setMomentum(momentum);
          }
          else
            iniState->getNucleon(id).setIsSpectator(false);
          if(id2 == id4)
          {
            iss6 >> pdg >> numDust >> px >> py >> pz >> e >> m >> x >> y >> z >> t;
            momentum.SetPxPyPzE(px, py, pz, e);
            iniState->getNucleon(id2).setMomentum(momentum);
          }
          else
            iniState->getNucleon(id2).setIsSpectator(false);
        }
        for(int i = 0; i < nOut - 2; i++)
          getline(inputFile, linestr);
      }
      // Skip the lines corresponding to collision products
      else
        for(int i = 0; i < nIn + nOut; i++)
          getline(inputFile, linestr);
    }
    iniState->setNColl(nColl);
    iniState->setNPart(nPart);

    // Identify final-state particles and store them
    //    cout << linestr << endl;
    //    cout << nIn << "\t" << nOut << endl;
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
        istringstream iss7(linestr);
        iss7 >> id >> pdg >> numDust >> px >> py >> pz >> e >> m >> x >> y >> z >> t;
        //                cout << linestr << endl;
        //        cout << "\t" << id << "\t" << pdg << "\t" << numDust << "\t" << px << "\t" << py << "\t" << pz << "\t"
        //        << e
        //             << "\t" << m << "\t" << x << "\t" << y << "\t" << z << "\t" << t << "\t" << endl;
        TLorentzVector momentum(px, py, pz, e);
        TLorentzVector position(x, y, z, t);
        event->AddParticle(id, pdg, status, parent, parentDecay, mate, decay, child, momentum, position, weight);
      }
    }
    if (iEvent == 8) cout << iEvent << "\t" << iniState->getNucleon(200).getId() << "\t" << iniState->getNucleon(201).isSpect() << "\t" << iniState->getNucleon(201).getMomentum().Pz() << endl;
    if(event->GetNpa() > nNucl)
      tree->Fill();
    getline(inputFile, linestr);
  }

  inputFile.close();
  header->SetNEvents(iEvent);
  header->Write();
  tree->Write();
  outputFile->Close();
}
