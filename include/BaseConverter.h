//
// Created by eugene on 7/30/19.
//

#ifndef MCINI_BASECONVERTER_H
#define MCINI_BASECONVERTER_H

#include <EventInitialState.h>
#include <TTree.h>
#include <TFile.h>

class BaseConverter {

 public:
  virtual ~BaseConverter() {}

  void resetEvent() {
    eventPtr_->id = 0;
    eventPtr_->nPart = 0;
    eventPtr_->nColl = 0;

    eventPtr_->nucleons.clear();
  }

  EventInitialState &getEvent() {
    return *eventPtr_;
  }

  IdType writeEvent() {
    getEvent().id = currentEventId++;
    eventTree_->Fill();
    resetEvent();
    return currentEventId;
  }

  template <class ...Args>
  Nucleon &addNucleon(Args... nucleonCtArgs) {
    getEvent().nucleons.emplace_back(std::forward(nucleonCtArgs)...);
    getEvent().nucleons.back().id = static_cast<IdType >(getEvent().nucleons.size() - 1);
  }

  static void registerCollision(Nucleon &n1, Nucleon &n2) {
    n1.collidedNucleonIndices.push_back(n2.id);
    n2.collidedNucleonIndices.push_back(n1.id);
  }


 private:
  IdType currentEventId{0};

  std::string outputFileName_{""};
  std::unique_ptr<TFile> outputFile_{};
  std::unique_ptr<TTree> eventTree_{};
  std::unique_ptr<EventInitialState> eventPtr_{new EventInitialState};



};

#endif //MCINI_BASECONVERTER_H