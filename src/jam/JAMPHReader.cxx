//
// Created by eugene on 8/24/19.
//

#include <iostream>
#include <fstream>
#include <memory>
#include <jam/JAMReader.h>
#include <cassert>

using namespace std;

namespace {

unique_ptr<ifstream> gJAMPHFile;

}

bool isValidInput() {
  return gJAMPHFile && gJAMPHFile->is_open();
}

void openJAMPHFile(const string &inputFileName) {
  gJAMPHFile = std::make_unique<ifstream>(inputFileName);

  if (isValidInput()) {
    cout << "File '" << inputFileName << "' opened successfully" << endl;
  } else {
    cerr << "File '" << inputFileName << "' is invalid" << endl;
  };
}

void parseNucleon(const string& line) {
  int pdgCode{0};
  // nucleus is either projectile (-1) or target (1) or unknown (0)
  short nucleus{0};
  float x{0.};
  float y{0.};
  float z{0.};

  float px{0.};
  float py{0.};
  float pz{0.};
  float energy{0.};
  float mass{0.};

  stringstream stream(line);

  stream >> pdgCode >> nucleus >>
         x >> y >> z >>
         px >> py >> pz >> energy >> mass;

  getEntity<Nucleon>()->setPdgId(pdgCode);
  getEntity<Nucleon>()->setNucleus(nucleus);
  getEntity<Nucleon>()->setPosition(TLorentzVector(x, y, z, 0.));
  getEntity<Nucleon>()->setMomentum(TLorentzVector(px, py, pz, energy));
  getEntity<Nucleon>()->Print();

}

void nucleonPostProcess() {
  auto numberOfNucleons = getEntity<EventInitialState>()->getNucleons().size();
  getEntity<Nucleon>()->setId(numberOfNucleons);
  getEntity<EventInitialState>()->addNucleon(*getEntity<Nucleon>());
}

bool parseNextJAMPHEvent() {
  assert(isValidInput());

  string line;

  bool isEventHeaderFound = false;

  streampos prevLinePosition{gJAMPHFile->tellg()};

  while (prevLinePosition = gJAMPHFile->tellg(), getline(*gJAMPHFile, line)) {
    if (line[0] == '#') {
      if (isEventHeaderFound) {
        cout << "event end" << endl;
        // undo read
        gJAMPHFile->seekg(prevLinePosition);
        break;
      }

      // this is event header
      cout << "event start" << endl;
      isEventHeaderFound = true;
      continue;
    }

    if (!isEventHeaderFound) {
      throw logic_error("no event header found");
    }

    auto nWords = countWordsInTheString(line);

    if (nWords == 10) {
      parseNucleon(line);
      nucleonPostProcess();
    }
  }

  getEntity<EventInitialState>()->Print();

  return isEventHeaderFound;
}


