//
// Created by eugene on 7/30/19.
//

#include <TBuffer.h>
#include <TClass.h>

#include <Nucleon.h>

ClassImp(Nucleon)

void Nucleon::Streamer(TBuffer &R__b) {
  if (R__b.IsReading()) {
    Nucleon::Class()->ReadBuffer(R__b, this);
    updateMomentumVector();
    updatePositionVector();
  } else {
    Nucleon::Class()->WriteBuffer(R__b, this);
  }
}