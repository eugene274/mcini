//
// Created by eugene on 8/6/19.
//

#ifndef MCINIDATA_JAMREADER_H
#define MCINIDATA_JAMREADER_H


enum class EParseStatus : ushort {
  kSuccess = 0,
  kNotInitialized,

  kNoInputFile,
  kMalformedInput,

};

#endif //MCINIDATA_JAMREADER_H
