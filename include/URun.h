#ifndef URUN_H
#define URUN_H

#include "TNamed.h"
#include "TString.h"


class URun : public TNamed {

 private:
  TString    fGenerator;     // Generator description
  TString    fComment;       // Run comment
  TString    fDecayer;       // Decayer description
  Int_t      fAProj;         // Projectile mass number
  Int_t      fZProj;         // Projectile charge
  Double32_t fPProj;         // Projectile momentum per nucleon (GeV)
  Int_t      fATarg;         // Target mass number
  Int_t      fZTarg;         // Target charge
  Double32_t fPTarg;         // Target momentum per nucleon (GeV)
  Double32_t fBMin;          // Minimum impact parameter
  Double32_t fBMax;          // Maximum impact parameter
  Int_t      fBWeight;       // Impact parameter weighting
                             // 0 for geometrical weights (bdb)
                             // 1 for flat distribution
  Double32_t fPhiMin;        // Event plane minimum angle (rad)
  Double32_t fPhiMax;        // Event plane maximum angle (rad)
  Double32_t fSigma;         // Cross-section (mb)
  Int_t      fNEvents;       // Requested number of events

 public:
  URun();
  URun(const char* generator, const char* comment, Int_t aProj,
       Int_t zProj, Double_t pProj, Int_t aTarg, Int_t zTarg,
       Double_t pTarg, Double_t bMin, Double_t bMax, Int_t bWeight,
       Double_t phiMin, Double_t phiMax, Double_t sigma, Int_t nEvents);
  virtual ~URun();
  void Print(Option_t* = "") const;
  void GetGenerator(TString& generator) {generator = fGenerator;}
  void GetComment(TString& comment)     {comment = fComment;}
  void GetDecayer(TString& decayer)     {decayer = fDecayer;}
  inline Int_t       GetAProj()   const {return fAProj;}
  inline Int_t       GetZProj()   const {return fZProj;}
  inline Double_t    GetPProj()   const {return fPProj;}
  inline Int_t       GetATarg()   const {return fATarg;}
  inline Int_t       GetZTarg()   const {return fZTarg;}
  inline Double_t    GetPTarg()   const {return fPTarg;}
  inline Double_t    GetBMin()    const {return fBMin;}
  inline Double_t    GetBMax()    const {return fBMax;}
  inline Int_t       GetBWeight() const {return fBWeight;}
  inline Double_t    GetPhiMax()  const {return fPhiMax;}
  inline Double_t    GetPhiMin()  const {return fPhiMin;}
  inline Double_t    GetSigma()   const {return fSigma;}
  inline Int_t       GetNEvents() const {return fNEvents;}
  Double_t    GetSqrtS();
  Double_t    GetNNSqrtS();
  Double_t    GetProjectileEnergy();
  Double_t    GetTargetEnergy();
  Double_t    GetBetaCM();
  Double_t    GetGammaCM();
  inline void SetNEvents(Int_t nEvents)   {fNEvents=nEvents;}
  inline void SetPProj  (Double_t pProj)  {fPProj=pProj;}
  inline void SetPTarg  (Double_t pTarg)  {fPTarg=pTarg;}
  inline void SetDecayer(TString decayer) {fDecayer=decayer;}

  ClassDef(URun,1);
};


#endif
