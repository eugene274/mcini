//
// Created by eugene on 8/6/19.
//

#ifndef MCINIDATA_JAMREADER_H
#define MCINIDATA_JAMREADER_H

#include <URun.h>
#include <UEvent.h>
#include <EventInitialState.h>
enum class EParseStatus : ushort {
  kSuccess = 0,
  kNotInitialized,

  kNoInputFile,
  kMalformedInput,

};

template<typename ... Elem>
using tuple_with_pointers = std::tuple<std::shared_ptr<Elem>...>;

template<typename T>
T *getEntity() {
  static tuple_with_pointers<URun, UEvent, UParticle, EventInitialState> dataModel;

  auto &entityPtr = std::get<std::shared_ptr<T>>(dataModel);

  if (!entityPtr) {
    entityPtr.reset(new T);
    Warning(__func__, "Created new '%s'", typeid(T).name());
  }

  return entityPtr.get();
}

EParseStatus parseJAMRunInfo(const std::string &pathToJAMRunInfo);

#endif //MCINIDATA_JAMREADER_H
