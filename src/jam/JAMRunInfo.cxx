//
// Created by eugene on 8/5/19.
//

#include <string>
#include <fstream>
#include <iostream>
/* Test for GCC > 4.9.0 */
#if __GNUC__ > 4 || \
  (__GNUC__ == 4 && (__GNUC_MINOR__ > 9 || \
  (__GNUC_MINOR__ == 9 && \
    __GNUC_PATCHLEVEL__ > 0)))
#include <regex>
#else
#include <boost/regex.hpp>
using boost::regex;
using boost::smatch;
#endif

#include <jam/JAMReader.h>
#include <URun.h>

using namespace std;

EParseStatus parseJAMRunInfo(const string &pathToJAMRunInfo) {
  cout << __func__ << endl;

  unsigned int aTarg{0};
  unsigned int aProj{0};
  unsigned int zTarg{0};
  unsigned int zProj{0};

  float pZTarg{0.};
  float pZProj{0.};

  float bMin{0.};
  float bMax{0.};

  unsigned int nEvents{0};

  EParseStatus parseStatus_{EParseStatus::kNotInitialized};


  ifstream jamRunInfoIF(pathToJAMRunInfo);

  if (!jamRunInfoIF.is_open()) {
    cerr << "File '" << pathToJAMRunInfo << "' is not available" << endl;
    parseStatus_ = EParseStatus::kNoInputFile;
    return parseStatus_;
  }

  const regex regexAZTargAndProj(R"(mass (\d+)\( *(\d+), *\d+\) ==> mass (\d+)\( *(\d+), *\d+\))");

  string inputBuffer{istreambuf_iterator<char>(jamRunInfoIF), istreambuf_iterator<char>()};

  smatch matchResults;

  if (regex_search(inputBuffer, matchResults, regexAZTargAndProj)) {
    if (matchResults.size() == 5) {
      try {
        aProj = stoi(matchResults.str(1));
        zProj = stoi(matchResults.str(2));
        aTarg = stoi(matchResults.str(3));
        zTarg = stoi(matchResults.str(4));

        cout << "Collision (" << aProj << "," << zProj << ") ==> (" << aTarg << ","
             << zTarg << ")" << endl;

        parseStatus_ = EParseStatus::kSuccess;
      } catch (invalid_argument& e) {
        cerr << e.what() << endl;
        parseStatus_ = EParseStatus::kMalformedInput;
      }
    } else {
      parseStatus_ = EParseStatus::kMalformedInput;
    }
  } else {
    parseStatus_ = EParseStatus::kMalformedInput;
  }

  if (parseStatus_ != EParseStatus::kSuccess) {
    return parseStatus_;
  }

  const regex regexPzTargAndProj(R"(p_z\S+\s*([\d\.-]+)\s+([\d\.-]+))");
  if (regex_search(inputBuffer, matchResults, regexPzTargAndProj)) {
    if (matchResults.size() == 3) {
      try {
        pZProj = stof(matchResults.str(1));
        pZTarg = stof(matchResults.str(2));

        cout << "pZ (Proj) = " << pZProj << " (A GeV/c) " << "pZ (Targ) = " << pZTarg
             << " (A GeV/c)" << endl;

        parseStatus_ = EParseStatus::kSuccess;
      } catch (invalid_argument &e) {
        cerr << e.what() << endl;
        parseStatus_ = EParseStatus::kMalformedInput;
      }
    } else {
      parseStatus_ = EParseStatus::kMalformedInput;
    }
  }

  if (parseStatus_ != EParseStatus::kSuccess) {
    return parseStatus_;
  }

  const regex regexImpactParameter(R"(([\d\.-]+) +< b < +([\d\.-]+) \(fm\))");
  if (regex_search(inputBuffer, matchResults, regexImpactParameter)) {
    if (matchResults.size() == 3) {
      bMin = stof(matchResults.str(1));
      bMax = stof(matchResults.str(2));

      parseStatus_ = EParseStatus::kSuccess;
    } else {
      parseStatus_ = EParseStatus::kMalformedInput;
    }
  } else {
    parseStatus_ = EParseStatus::kMalformedInput;
  }

  if (parseStatus_ != EParseStatus::kSuccess) {
    return parseStatus_;
  }

  const regex regexNEvents(R"(# of event\s+=\s+(\d+))");
  if (regex_search(inputBuffer, matchResults, regexNEvents)) {
    if (matchResults.size() == 2) {
      nEvents = stoi(matchResults.str(1));

      parseStatus_ = EParseStatus::kSuccess;
    } else {
      parseStatus_ = EParseStatus::kMalformedInput;
    }
  } else {
    parseStatus_ = EParseStatus::kMalformedInput;
  }

  *getEntity<URun>() = URun(
      "JAM", "",
      aProj, zProj, pZProj,
      aTarg, zTarg, pZTarg,
      bMin, bMax, 0,
      0., 0., 0,
      nEvents
      );

  return parseStatus_;
}

