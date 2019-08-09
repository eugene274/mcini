#include <iostream>
#include <sstream>
#include <memory>

#include <TFile.h>
#include <TTree.h>
#include <fstream>

#include <jam/JAMReader.h>

#include <URun.h>
#include <UEvent.h>
#include <UParticle.h>

namespace {
using std::ifstream;
using std::stringstream;
using std::string;
using std::cout;
using std::endl;
}

enum class ELineType {
  kRunHeader,
  kEventHeader,
  kParticle,
  kNotInitializedYet,
  kUnknown
};

TFile *gOutputFile{nullptr};
TTree *gEventTree{nullptr};

unsigned int countWordsInTheString(const std::string &str) {
  std::stringstream stream(str);
  std::string word;
  unsigned int words = 0;
  while (stream >> word) {
    ++words;
  }
  return words;
}

void parseRunHeader(const string &line) {
  stringstream stream(line);

  string hashStr;
  stream >> hashStr;
  if (hashStr != "#") {
    throw std::runtime_error("Malformed run header: must start with hash");
  }

  unsigned int nEvents{0};
  stream >> nEvents;

  getEntity<URun>()->SetNEvents(nEvents);
}

void parseEventHeader(const string &line) {
  stringstream stream(line);

  string hashStr;
  stream >> hashStr;
  if (hashStr != "#") {
    throw std::runtime_error("Malformed event header: must start with hash");
  }

  unsigned int eventId{0};
  unsigned int nParticles{0};
  unsigned int nBaryons{0};
  unsigned int nMesons{0};

  float impactParameter{-1.};

  unsigned int nParticipants{0};
  unsigned int nCollisions{0};

  stream
      >> eventId >> nParticles >> nBaryons >> nMesons >> impactParameter >> nParticipants >> nCollisions;

  getEntity<UEvent>()->Clear();
  getEntity<UEvent>()->SetParameters(eventId, impactParameter, 0., 0, 0, 0.);

  getEntity<EventInitialState>()->clear();
  getEntity<EventInitialState>()->setId(eventId);
  getEntity<EventInitialState>()->setNPart(nParticipants);
  getEntity<EventInitialState>()->setNColl(nCollisions);
}

void parseParticle(const string &line) {
  stringstream stream(line);

  int par0{-1};
  int pdgCode{0};
  int par2{-1};

  float mass{0};
  float px{0};
  float py{0};
  float pz{0};
  float energy{0};
  float x{0};
  float y{0};
  float z{0};
  float time{0};

  stream
      >> par0 >> pdgCode >> par2
      >> mass >> px >> py >> pz >> energy
      >> x >> y >> z >> time;

  getEntity<UParticle>()->Clear();
  getEntity<UParticle>()->SetPdg(pdgCode);
  getEntity<UParticle>()->SetPosition(x, y, z, time);
  getEntity<UParticle>()->SetMomentum(px, py, pz, energy);
}

void particlePostProcess() {
  getEntity<UEvent>()->AddParticle(*getEntity<UParticle>());
}

void previousEventPostProcess() {
  getEntity<UEvent>()->Print();

  if (getEntity<EventInitialState>()->getNColl() != 0) {
    gEventTree->Fill();
  }

}

int main(int argc, char **argv) {
  std::cout << "go!" << std::endl;

  string inputFileName{"phase.dat"};
  if (argc > 1) {
    inputFileName = string(argv[1]);
  }

  string jamOutputFileName{"jamOutput.root"};
  if (argc > 2) {
    jamOutputFileName = string(argv[2]);
  }

  string runInfoFileName{"JAMRUN.DAT"};
  if (argc > 3) {
    runInfoFileName = string(argv[3]);
  }

  ifstream inputFile(argv[1]);
  gOutputFile = TFile::Open(jamOutputFileName.c_str(), "recreate");

  gEventTree = new TTree("events", "JAM events");
  gEventTree->Branch("event", getEntity<UEvent>());
  gEventTree->Branch("iniState", getEntity<EventInitialState>());

  bool hasPreviousEvent{false};
  unsigned int nEvents{0};

  string line;
  while (std::getline(inputFile, line)) {
    unsigned int nWords = countWordsInTheString(line);

    ELineType currentLineType{ELineType::kNotInitializedYet};

    switch (nWords) {
      case 6:currentLineType = ELineType::kRunHeader;
        parseRunHeader(line);
        break;
      case 9:currentLineType = ELineType::kEventHeader;
        if (hasPreviousEvent) previousEventPostProcess();
        parseEventHeader(line);
        hasPreviousEvent = true;
        nEvents++;
        break;
      case 13:currentLineType = ELineType::kParticle;
        parseParticle(line);
        particlePostProcess();
        break;
      default:currentLineType = ELineType::kUnknown;
    }
  }

  if (nEvents != getEntity<URun>()->GetNEvents()) {
    Error(__func__, "nEvents(%d) != actual(%d)", nEvents, getEntity<URun>()->GetNEvents());
    return 1;
  }

  gEventTree->Write();

  if (parseJAMRunInfo(runInfoFileName) == EParseStatus::kSuccess) {
    getEntity<URun>()->Print();
    getEntity<URun>()->Write();
  }

  gOutputFile->Close();

  return 0;
}