//
// Created by eugene on 7/30/19.
//

#ifndef MCININUCLEONH
#define MCININUCLEONH

#include <TLorentzVector.h>
#include <vector>
#include "BasicTypes.h"

class Nucleon : public TObject {

 public:
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
  double getMass() const {
    return mass;
  }
  void setMass(double mass) {
    Nucleon::mass = mass;
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
  bool isSpectator1() const {
    return isSpectator;
  }
  void setIsSpectator(bool isSpectator) {
    Nucleon::isSpectator = isSpectator;
  }
  const std::vector<IdType> &getCollidedNucleiIndices() const {
    return collidedNucleiIndices;
  }
  void setCollidedNucleiIndices(const std::vector<IdType> &collidedNucleiIndices) {
    Nucleon::collidedNucleiIndices = collidedNucleiIndices;
  }

 private:
  IdType id;

  int pdgId{-1};

  double mass;
  TLorentzVector momentum{};
  TLorentzVector position{};

  bool isSpectator{false};
  std::vector <IdType > collidedNucleiIndices{};

  ClassDef(Nucleon, 1);
};


#endif //MCININUCLEONH
