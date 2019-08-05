//
// Created by eugene on 8/5/19.
//

#include <string>
#include <fstream>
#include <iostream>
#include <regex>

enum class EParseStatus {
  kNotYetParsed,
  kNoInputFile,
  kMalformedInput,
  kSuccess
};

struct JAMRunInfo_st {

  unsigned int aTarg_{0};
  unsigned int aProj_{0};
  unsigned int zTarg_{0};
  unsigned int zProj_{0};

  float pZTarg_{0.};
  float pZProj_{0.};

  EParseStatus parseStatus_{EParseStatus::kNotYetParsed};
};

using namespace std;

JAMRunInfo_st parseJAMRunInfo(const string &pathToJAMRunInfo) {
  cout << __func__ << endl;
  JAMRunInfo_st parseResult{};

  ifstream jamRunInfoIF(pathToJAMRunInfo);

  if (!jamRunInfoIF.is_open()) {
    cerr << "File '" << pathToJAMRunInfo << "' is not available" << endl;
    parseResult.parseStatus_ = EParseStatus::kNoInputFile;
    return parseResult;
  }

  const regex regexAZTargAndProj(R"(mass (\d+)\( *(\d+), *\d+\) ==> mass (\d+)\( *(\d+), *\d+\))");

  string inputBuffer{istreambuf_iterator<char>(jamRunInfoIF), istreambuf_iterator<char>()};

  smatch matchResults;

  if (regex_search(inputBuffer, matchResults, regexAZTargAndProj)) {
    if (matchResults.size() == 5) {
      try {
        parseResult.aProj_ = stoi(matchResults.str(1));
        parseResult.zProj_ = stoi(matchResults.str(2));
        parseResult.aTarg_ = stoi(matchResults.str(3));
        parseResult.zTarg_ = stoi(matchResults.str(4));

        cout << "Collision (" << parseResult.aProj_ << "," << parseResult.zProj_ << ") ==> (" << parseResult.aTarg_ << ","
             << parseResult.zTarg_ << ")" << endl;

        parseResult.parseStatus_ = EParseStatus::kSuccess;
      } catch (invalid_argument& e) {
        cerr << e.what() << endl;
        parseResult.parseStatus_ = EParseStatus::kMalformedInput;
      }
    } else {
      parseResult.parseStatus_ = EParseStatus::kMalformedInput;
    }
  } else {
    parseResult.parseStatus_ = EParseStatus::kMalformedInput;
  }

  if (parseResult.parseStatus_ != EParseStatus::kSuccess) {
    return parseResult;
  }

  const regex regexPzTargAndProj(R"(p_z\S+\s*([\d\.-]+)\s+([\d\.-]+))");
  if (regex_search(inputBuffer, matchResults, regexPzTargAndProj)) {
    if (matchResults.size() == 3) {
      try {
        parseResult.pZProj_ = stof(matchResults.str(1));
        parseResult.pZTarg_ = stof(matchResults.str(2));

        cout << "pZ (Proj) = " << parseResult.pZProj_ << " (A GeV/c) " << "pZ (Targ) = " << parseResult.pZTarg_
             << " (A GeV/c)" << endl;

        parseResult.parseStatus_ = EParseStatus::kSuccess;
      } catch (invalid_argument &e) {
        cerr << e.what() << endl;
        parseResult.parseStatus_ = EParseStatus::kMalformedInput;
      }
    } else {
      parseResult.parseStatus_ = EParseStatus::kMalformedInput;
    }
  }

  return parseResult;
}

// for the internal tests only
int main(int argc, char **argv) {

  parseJAMRunInfo(argv[1]);

  return 0;
}