#ifndef UEVENT_H
#define UEVENT_H

#include "TObject.h"
#include "TLorentzVector.h"

class UParticle;

class TString;
class TClonesArray;

class UEvent : public TObject {

 private:
  Int_t         fEventNr;      // Event number
  Double_t      fB;            // Impact parameter (fm)
  Double_t      fPhi;          // Event plane angle
  Int_t         fNes;          // Number of event steps
  Int_t         fStepNr;       // Event step number
  Double_t      fStepT;        // Event step time
  Int_t         fNpa;          // Number of particles
  TClonesArray* fParticles;    // Array of particles
  
 public:
  UEvent();
  UEvent(const UEvent& right);
  virtual ~UEvent();
  void Print(Option_t* option = "") const;
  inline Int_t    GetEventNr() const {return fEventNr;}
  inline Double_t GetB()       const {return fB;}
  inline Double_t GetPhi()     const {return fPhi;}
  inline Int_t    GetNes()     const {return fNes;}
  inline Int_t    GetStepNr()  const {return fStepNr;}
  inline Double_t GetStepT()   const {return fStepT;}
  inline Int_t    GetNpa()     const {return fNpa;}
  inline TClonesArray* GetParticleList() const {return fParticles;}
  UParticle* GetParticle(Int_t index) const;
  void SetParameters(Int_t eventNr, Double_t b, Double_t phi, Int_t nes,
		     Int_t stepNr, Double_t stepT);
  inline void SetEventNr(Int_t eventNr)    {fEventNr = eventNr;}
  inline void SetB      (Double_t b)       {fB = b;}
  inline void SetPhi    (Double_t phi)     {fPhi = phi;}
  inline void SetNes    (Int_t nes)        {fNes = nes;}
  inline void SetStepNr (Int_t stepNr)     {fStepNr = stepNr;}
  inline void SetStepT  (Double_t stepT)   {fStepT = stepT;}
  void AddParticle(Int_t index, Int_t pdg, Int_t status,
		   Int_t parent, Int_t parentDecay,
		   Int_t mate, Int_t decay, Int_t child[2],
		   Double_t px, Double_t py, Double_t pz, Double_t e,
		   Double_t x, Double_t y, Double_t z, Double_t t,
		   Double_t weight);
  void AddParticle(Int_t index, Int_t pdg, Int_t status,
		   Int_t parent, Int_t parentDecay,
		   Int_t mate, Int_t decay, Int_t child[2],
		   TLorentzVector mom, TLorentzVector pos,
		   Double_t weight);
  void AddParticle(const UParticle& particle);
  void Clear(Option_t* = "");
  void RemoveAt(Int_t i);

 private:
  UEvent operator=(const UEvent&);

  ClassDef(UEvent, 1);
};


#endif
