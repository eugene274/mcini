//
// Created by eugene on 7/30/19.
//

#ifndef MCINI_EVENTINITIALSTATE_H
#define MCINI_EVENTINITIALSTATE_H

#include <vector>
#include <Rtypes.h>

#include "BasicTypes.h"
#include "Nucleon.h"

class EventInitialState {

 public:
  IdType getId() const {
    return id;
  }
  void setId(IdType id) {
    EventInitialState::id = id;
  }
  unsigned int getNColl() const {
    return nColl;
  }
  void setNColl(unsigned int nColl) {
    EventInitialState::nColl = nColl;
  }
  unsigned int getNPart() const {
    return nPart;
  }
  void setNPart(unsigned int nPart) {
    EventInitialState::nPart = nPart;
  }
  const std::vector<Nucleon> &getNucleons() const {
    return nucleons;
  }
  void setNucleons(const std::vector<Nucleon> &nucleons) {
    EventInitialState::nucleons = nucleons;
  }

 private:
  IdType id;

  unsigned int nColl;
  unsigned int nPart;

  std::vector <Nucleon> nucleons;



  ClassDef(EventInitialState, 1);
};

#endif //MCINI_EVENTINITIALSTATE_H
