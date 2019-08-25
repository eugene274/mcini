//
// Created by eugene on 8/6/19.
//

#ifndef MCINIDATA_JAMREADER_H
#define MCINIDATA_JAMREADER_H

#include <URun.h>
#include <UEvent.h>
#include <EventInitialState.h>

#include <string>
#include <sstream>

enum class EParseStatus : ushort {
  kSuccess = 0,
  kNotInitialized,

  kNoInputFile,
  kMalformedInput,

};

template<typename T>
T *getEntity() {
  static std::shared_ptr<T> entityPtr{};

  if (!entityPtr) {
    entityPtr.reset(new T);
    Info(__func__, "Created new '%s'", typeid(T).name());
  }

  return entityPtr.get();
}

unsigned int countWordsInTheString(const std::string &str);

EParseStatus parseJAMRunInfo(const std::string &pathToJAMRunInfo);


void openJAMPHFile(const std::string &inputFileName);
bool parseNextJAMPHEvent();

#endif //MCINIDATA_JAMREADER_H
