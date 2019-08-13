#!/bin/bash

# absolute path to JAM executable
#JAM_EXE=/lustre/nyx/cbm/users/kashirin/soft/JAM/jam-1.9043/main/jamexe
JAM_EXE=${JAM_EXE:?NO JAM executable found}
# absolute path to mcini:JAMReader executable
#CONVERTER_EXE=/lustre/nyx/cbm/users/kashirin/soft/mcini/build/JAMReader
CONVERTER_EXE=${CONVERTER_EXE:?NO JAMReader executable found}
# absolute path to config template
CFG_TMPL=$PWD/jam.cfg.template
#CFG_TMPL=${CFG_TMPL:?No jam.cfg.template found}

WD=$PWD

LOG_FILE=$WD/jam.log

output_file=${1:-unigen.root}
shift

SEED=$(head -1 /dev/urandom | od -N 1 | awk '{ print $2 }')
NEVENTS=10
PROJ=197Au
TARG=197Au
# kinetic energy of beam nucleon in LAB
INCIDENT_KINE=29.7gev
B_MAX=20

JAMEOS=${JAMEOS:?No JAM EOS file is set}
#JAMEOS=/lustre/nyx/cbm/users/kashirin/soft/JAM/JAMEOS/eos_MF_HGJAM.dat

FRMULT=${FRMULT:?No frmult.dat file is set}
#FRMULT=/lustre/nyx/cbm/users/kashirin/soft/JAM/JAMEOS/frmult.dat

echo go!

rm -v phase.dat phase.dat.gz jam.cfg
rm -v *.{dat,DAT}

sed\
        -e 's!@NEVENTS@!'$NEVENTS'!g'\
        -e 's!@PROJ@!'$PROJ'!g'\
        -e 's!@TARG@!'$TARG'!g'\
        -e 's!@SEED@!'$SEED'!g'\
        -e 's!@INCIDENT_KINE@!'$INCIDENT_KINE'!g'\
        -e 's!@B_MAX@!'$B_MAX'!g'\
        -e 's!@JAMEOS@!'$JAMEOS'!g'\
        -e 's!@FRMULT@!'$FRMULT'!g'\
        $CFG_TMPL > $WD/jam.cfg


$JAM_EXE |& tee $LOG_FILE

if ! [[ -f phase.dat.gz && -f JAMRUN.DAT ]]; then
    echo No gunzipped output found
    exit 1
fi

gunzip phase.dat.gz

$CONVERTER_EXE phase.dat unigen.root JAMRUN.DAT

rsync -v unigen.root $output_file
