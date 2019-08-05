//
// Created by eugene on 7/30/19.
//

#ifndef MCININUCLEONH
#define MCININUCLEONH

#include <TLorentzVector.h>
#include <vector>
#include "BasicTypes.h"

class BaseConverter;

class Nucleon : public TObject {

public:
  Nucleon() {}
  Nucleon(int pdgId, const TLorentzVector &momentum, const TLorentzVector &position, bool isSpectator)
      : pdgId(pdgId), momentum(momentum), position(position), isSpectator(isSpectator) {}
  Nucleon(IdType id,
          int pdgId,
          const TLorentzVector &momentum,
          const TLorentzVector &position,
          bool isSpectator,
          const std::vector<IdType> &collidedNucleonIndices)
      : id(id),
        pdgId(pdgId),
        momentum(momentum),
        position(position),
        isSpectator(isSpectator),
        collidedNucleonIndices(collidedNucleonIndices) {}
  IdType getId() const {
    return id;
  }
  void setId(IdType id) {
    Nucleon::id = id;
  }
  int getPdgId() const {
    return pdgId;
  }
  void setPdgId(int pdgId) {
    Nucleon::pdgId = pdgId;
  }
  const TLorentzVector &getMomentum() const {
    return momentum;
  }
  void setMomentum(const TLorentzVector &momentum) {
    Nucleon::momentum = momentum;
  }
  const TLorentzVector &getPosition() const {
    return position;
  }
  void setPosition(const TLorentzVector &position) {
    Nucleon::position = position;
  }
  bool isSpect() const {
    return isSpectator;
  }
  void setIsSpectator(bool isSpectator) {
    Nucleon::isSpectator = isSpectator;
  }
  const std::vector<IdType> &getCollidedNucleonIndices() const {
    return collidedNucleonIndices;
  }
  void setCollidedNucleonIndices(const std::vector<IdType> &collidedNucleonIndices) {
    Nucleon::collidedNucleonIndices = collidedNucleonIndices;
  }
  void addCollidedNucleonIndex(int index) {
    collidedNucleonIndices.push_back(index);
    isSpectator = false;
  }

  void Clear(Option_t* = "") {
    id=0;
    pdgId=-1;
    momentum.SetXYZT(0, 0, 0, 999);
    position.SetXYZT(0, 0, 0, 999);
    isSpectator=true;
    collidedNucleonIndices.clear();
  }

private:
  friend class BaseConverter;
  IdType id;

  int pdgId{-1};

  TLorentzVector momentum{0, 0, 0, 999};
  TLorentzVector position{0, 0, 0, 999};

  bool isSpectator{true};
  std::vector <IdType > collidedNucleonIndices{};

  ClassDef(Nucleon, 1)
};


#endif //MCININUCLEONH
