//
// Created by eugene on 7/30/19.
//

#ifndef MCININUCLEONH
#define MCININUCLEONH

#include <vector>
#include <iostream>

#include <TLorentzVector.h>
#include <iomanip>
#include "BasicTypes.h"

class Nucleon : public TObject {

 public:
  Nucleon() = default;
  Nucleon(int pdgId, const TLorentzVector &momentum, const TLorentzVector &position, bool isSpectator)
      : pdgId(pdgId), momentum(momentum), position(position), isSpectator(isSpectator) {
    setMomentum(momentum);
    setPosition(position);
  }
  Nucleon(IdType id,
          int pdgId,
          const TLorentzVector &momentum,
          const TLorentzVector &position,
          bool isSpectator,
          const std::vector<IdType> &collidedNucleonIndices)
      : id(id),
        pdgId(pdgId),
        isSpectator(isSpectator),
        collidedNucleonIndices(collidedNucleonIndices) {
    setMomentum(momentum);
    setPosition(position);
  }
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
  short getNucleus() const {
    return nucleus;
  }
  void setNucleus(short nucleus) {
    Nucleon::nucleus = nucleus;
  }
  const TLorentzVector &getMomentum() const {
    return momentum;
  }
  void setMomentum(const TLorentzVector &momentum) {
    setMomentum(momentum.Px(), momentum.Py(), momentum.Pz(), momentum.E());
  }
  void setMomentum(Float_t px, Float_t py, Float_t pz, Float_t energy) {
    this->px = px;
    this->py = py;
    this->pz = pz;
    this->energy = energy;
    updateMomentumVector();
  }
  const TLorentzVector &getPosition() const {
    return position;
  }
  void setPosition(const TLorentzVector &position) {
    setPosition(position.X(), position.Y(), position.Z(), position.T());
  }
  void setPosition(Float_t x, Float_t y, Float_t z, Float_t t) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->t = t;
    updatePositionVector();
  }
  bool isSpect() const {
    return isSpectator;
  }
  void setIsSpectator(bool isSpectator = true) {
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
  }

  void Clear(Option_t * = 0) override {
    id = 0;
    pdgId = -1;
    momentum.SetXYZT(0, 0, 0, 999);
    position.SetXYZT(0, 0, 0, 999);
    isSpectator = true;
    collidedNucleonIndices.clear();
  }

  void Print(Option_t *option = 0) const override {
    using std::cout;
    using std::endl;

    cout.setf(std::ostream::scientific);
    cout.precision(5);

    cout
        << "---------------------------" << endl
        << "Nucleon   #  " << std::setw(4) << id << endl
        << "PDG code   : " << std::setw(12) << pdgId << endl
        << "Nucleus    : " << (nucleus == -1 ? "projectile" : "target") << endl
        << "r = (" << position.T() << "," << position.X() << "," << position.Y() << "," << position.Z() << ") fm"
        << endl
        << "p = (" << momentum.E() << "," << momentum.Px() << "," << momentum.Py() << "," << momentum.Pz() << ") GeV/c"
        << endl;

  }

 private:
  void updateMomentumVector() {
    momentum.SetPxPyPzE(px, py, pz, energy);
  }

  void updatePositionVector() {
    position.SetXYZT(x, y, z, t);
  }

  IdType id{};

  int pdgId{-1};

  Short_t nucleus{0};

  Float16_t x{0.};
  Float16_t y{0.};
  Float16_t z{0.};
  Float16_t t{0.};

  TLorentzVector position{0, 0, 0, 999}; //!

  Float16_t px{0.};
  Float16_t py{0.};
  Float16_t pz{0.};
  Float16_t energy{0.};

  TLorentzVector momentum{0, 0, 0, 999}; //!

  bool isSpectator{true};
  std::vector<IdType> collidedNucleonIndices{};

 ClassDefOverride(Nucleon, 2)
};

#endif //MCININUCLEONH
