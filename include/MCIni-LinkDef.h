//
// Created by eugene on 7/30/19.
//

// some C++ header definition
#if defined(__ROOTCLING__) || defined(__MAKECINT__)
// turns off dictionary generation for all
#pragma link off all class;
#pragma link off all function;
#pragma link off all global;
#pragma link off all typedef;

#pragma link C++ class Nucleon+;
#pragma link C++ class EventInitialState+;
#pragma link C++ class URun+;
#pragma link C++ class UEvent+;
#pragma link C++ class UParticle+;

#endif
