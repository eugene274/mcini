//
// Created by eugene on 7/30/19.
//

#ifndef MCINI_EVENTINITIALSTATE_H
#define MCINI_EVENTINITIALSTATE_H

#include <vector>
#include <Rtypes.h>

#include "BasicTypes.h"
#include "Nucleon.h"

class BaseConverter;

class EventInitialState : public TObject {

public:
  EventInitialState() = default;
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
  void Clear(Option_t* = "") override {
    id=0;
    nucleons.clear();
    nColl=0;
    nPart=0;
  }
  void addNucleon (Nucleon &nucleon) {
    nucleons.push_back(nucleon);
  }
  Nucleon &getNucleon (int id) {
    return nucleons.at(id-1);
  }
  

private:
  friend class BaseConverter;
  IdType id{0};

  unsigned int nColl{0};
  unsigned int nPart{0};

  std::vector <Nucleon> nucleons{0};



  ClassDef(EventInitialState, 1)
};

#endif //MCINI_EVENTINITIALSTATE_H
