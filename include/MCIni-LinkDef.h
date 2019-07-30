//
// Created by eugene on 7/30/19.
//

// some C++ header definition
#ifdef __ROOTCLING__
// turns off dictionary generation for all
#pragma link off all class;
#pragma link off all function;
#pragma link off all global;
#pragma link off all typedef;

#pragma link C++ class Nucleon+;
#pragma link C++ class EventInitialState+;

#endif
