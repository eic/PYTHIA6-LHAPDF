#!/bin/sh

echo START

date

number=$1

Q2MIN=$2
Q2MAX=$3

ENRG=$4
PNRG=$5

WRKDIR="/gpfs/mnt/gpfs02/eic/bpage/home/EPIC/officialSimu/pythia6/sidis_minbias/condorLogs"
STEERDEST="/gpfs/mnt/gpfs02/eic/bpage/home/EPIC/officialSimu/pythia6/sidis_minbias/STEERFILES"
TREEDEST="/gpfs/mnt/gpfs02/eic/bpage/home/EPIC/officialSimu/pythia6/sidis_minbias/TREES"
TXTDEST="/gpfs/mnt/gpfs02/eic/bpage/home/EPIC/officialSimu/pythia6/sidis_minbias/TXTFILES"
LOGDEST="/gpfs/mnt/gpfs02/eic/bpage/home/EPIC/officialSimu/pythia6/sidis_minbias/LOGFILES"
HEPMCDEST="/gpfs/mnt/gpfs02/eic/bpage/home/EPIC/officialSimu/pythia6/sidis_minbias/HEPMCFILES"

# Change to Working Directory
cd $WRKDIR

# Write Steering File
echo "pythia_ep_noradcor_${ENRG}x${PNRG}_q2_${Q2MIN}_${Q2MAX}_run${number}.txt" 1>>steeringFile_${number}.txt #! output file name
echo "11" 1>>steeringFile_${number}.txt                #! lepton beam type
echo "$PNRG, $ENRG" 1>>steeringFile_${number}.txt      #! proton and electron beam energy
echo "1000000,1" 1>>steeringFile_${number}.txt         #! Number of events
echo "1e-09, 0.99" 1>>steeringFile_${number}.txt       #! xmin and xmax
echo "1e-09, 1.00" 1>>steeringFile_${number}.txt       #! ymin and ymax
echo "$Q2MIN,$Q2MAX" 1>>steeringFile_${number}.txt     #! Q2min and Q2max
echo "F2PY,1998" 1>>steeringFile_${number}.txt         #! F2-Model, R-Parametrisation
echo "0" 1>>steeringFile_${number}.txt                 #! switch for rad corrections; 0:no, 1:yes, 2:gen.lookup table
echo "1" 1>>steeringFile_${number}.txt                 #! Pythia-Model = 0 standard GVMD generation in Pythia-x and Q2; = 1 GVMD model with generation in y and Q2 as for radgen
echo "1,1" 1>>steeringFile_${number}.txt               #! A-Tar and Z-Tar
echo "1,1" 1>>steeringFile_${number}.txt               #! nuclear pdf parameter1: nucleon mass number A, charge number Z
echo "201" 1>>steeringFile_${number}.txt               #! nuclear pdf parameter2: correction order x*100+y x= 1:LO, 2:NLO y:error set
#! PMAS(4,1)=1.27  ! charm mass
echo "MSEL=2" 1>>steeringFile_${number}.txt
echo "MSTP(13)=1" 1>>steeringFile_${number}.txt
echo "MSTP(14)=30" 1>>steeringFile_${number}.txt
echo "MSTP(15)=0" 1>>steeringFile_${number}.txt
echo "MSTP(16)=1" 1>>steeringFile_${number}.txt
echo "MSTP(17)=4" 1>>steeringFile_${number}.txt #! MSTP 17=6 is the R-rho measured as by hermes, =4 Default
echo "MSTP(18)=3" 1>>steeringFile_${number}.txt
echo "MSTP(19)=1" 1>>steeringFile_${number}.txt #! Hermes MSTP-19=1 different Q2 suppression, default = 4
echo "MSTP(20)=3" 1>>steeringFile_${number}.txt #! Hermes MSTP(20)=4 , default MSTP(20)=3
echo "MSTP(32)=8" 1>>steeringFile_${number}.txt
echo "MSTP(38)=4" 1>>steeringFile_${number}.txt
#!MSTP(51)=10042 ! if pdflib is linked than non pythia-pdfs are available, like MSTP(51)=4046 
echo "MSTP(51)=7" 1>>steeringFile_${number}.txt
echo "MSTP(52)=1" 1>>steeringFile_${number}.txt   #! ---> pdflib used MSTP   52=2
echo "MSTP(53)=3" 1>>steeringFile_${number}.txt
echo "MSTP(54)=1" 1>>steeringFile_${number}.txt
echo "MSTP(55)=5" 1>>steeringFile_${number}.txt
echo "MSTP(56)=1" 1>>steeringFile_${number}.txt
echo "MSTP(57)=1" 1>>steeringFile_${number}.txt
echo "MSTP(58)=5" 1>>steeringFile_${number}.txt
echo "MSTP(59)=1" 1>>steeringFile_${number}.txt
echo "MSTP(60)=7" 1>>steeringFile_${number}.txt
echo "MSTP(61)=2" 1>>steeringFile_${number}.txt
echo "MSTP(71)=1" 1>>steeringFile_${number}.txt
echo "MSTP(81)=0" 1>>steeringFile_${number}.txt
echo "MSTP(82)=1" 1>>steeringFile_${number}.txt
echo "MSTP(91)=1" 1>>steeringFile_${number}.txt
echo "MSTP(92)=3" 1>>steeringFile_${number}.txt      #! hermes MSTP(92)=4
echo "MSTP(93)=1" 1>>steeringFile_${number}.txt
echo "MSTP(94)=2" 1>>steeringFile_${number}.txt     #! D=3 modified to 2 based on Mark tune
echo "MSTP(101)=3" 1>>steeringFile_${number}.txt
echo "MSTP(102)=1" 1>>steeringFile_${number}.txt
echo "MSTP(111)=1" 1>>steeringFile_${number}.txt
echo "MSTP(121)=0" 1>>steeringFile_${number}.txt
#! ----------- Now all the PARPs -----------
echo "PARP(2)=5." 1>>steeringFile_${number}.txt #!min CMS energy allowed for event as a whole
echo "PARP(13)=1" 1>>steeringFile_${number}.txt
echo "PARP(18)=0.40" 1>>steeringFile_${number}.txt #! hermes PARP(18)=0.17
echo "PARP(81)=1.9" 1>>steeringFile_${number}.txt
echo "PARP(89)=1800" 1>>steeringFile_${number}.txt
echo "PARP(90)=0.16" 1>>steeringFile_${number}.txt
echo "PARP(91)=0.32" 1>>steeringFile_${number}.txt #! rms kt D=2.0, Hermes=0.4, tuned by Mark 0.32
echo "PARP(93)=5." 1>>steeringFile_${number}.txt
echo "PARP(97)=6.0" 1>>steeringFile_${number}.txt  #! D=1.0, tuned by Mark 6.0
echo "PARP(99)=0.32" 1>>steeringFile_${number}.txt
echo "PARP(100)=5" 1>>steeringFile_${number}.txt
echo "PARP(102)=0.28" 1>>steeringFile_${number}.txt
echo "PARP(103)=1.0" 1>>steeringFile_${number}.txt
echo "PARP(104)=0.8" 1>>steeringFile_${number}.txt
echo "PARP(111)=2." 1>>steeringFile_${number}.txt
echo "PARP(161)=3.00" 1>>steeringFile_${number}.txt
echo "PARP(162)=24.6" 1>>steeringFile_${number}.txt
echo "PARP(163)=18.8" 1>>steeringFile_${number}.txt
echo "PARP(164)=11.5" 1>>steeringFile_${number}.txt
echo "PARP(165)=0.47679" 1>>steeringFile_${number}.txt
echo "PARP(166)=0.67597" 1>>steeringFile_${number}.txt #! PARP165/166 are linked to MSTP17 as R_rho of HERMES is used
#! PARP(166)=0.5    
#! ----------- Now come all the switches for Jetset -----------
echo "PARJ(1)=0.100" 1>>steeringFile_${number}.txt
echo "PARJ(2)=0.300" 1>>steeringFile_${number}.txt
echo "PARJ(3)=0.4" 1>>steeringFile_${number}.txt
echo "PARJ(11)=0.5" 1>>steeringFile_${number}.txt
echo "PARJ(12)=0.6" 1>>steeringFile_${number}.txt
echo "PARJ(21)=0.32" 1>>steeringFile_${number}.txt #! fragpt width D=0.36, Hermes=0.40, Mark=0.32
echo "PARJ(32)=1.0" 1>>steeringFile_${number}.txt
echo "PARJ(33)=0.80" 1>>steeringFile_${number}.txt
echo "PARJ(41)=0.30" 1>>steeringFile_${number}.txt
echo "PARJ(42)=0.58" 1>>steeringFile_${number}.txt
echo "PARJ(45)=0.5" 1>>steeringFile_${number}.txt
echo "PARJ(170)=0.32" 1>>steeringFile_${number}.txt #!pt for remnant in FF D=0.36, Mark=0.32
#!----------------------------------------------------------------------
echo "MSTJ(1)=1" 1>>steeringFile_${number}.txt
echo "MSTJ(12)=1" 1>>steeringFile_${number}.txt
echo "MSTJ(45)=5" 1>>steeringFile_${number}.txt
echo "MSTU(16)=2" 1>>steeringFile_${number}.txt
echo "MSTU(112)=5" 1>>steeringFile_${number}.txt
echo "MSTU(113)=5" 1>>steeringFile_${number}.txt
echo "MSTU(114)=5" 1>>steeringFile_${number}.txt
#! ----------- Now all the CKINs for pythia ----------
echo "CKIN(1)=1." 1>>steeringFile_${number}.txt
echo "CKIN(2)=-1." 1>>steeringFile_${number}.txt
echo "CKIN(3)=0." 1>>steeringFile_${number}.txt
echo "CKIN(4)=-1." 1>>steeringFile_${number}.txt
echo "CKIN(5)=1.00" 1>>steeringFile_${number}.txt
echo "CKIN(6)=1.00" 1>>steeringFile_${number}.txt
echo "CKIN(7)=-10." 1>>steeringFile_${number}.txt
echo "CKIN(8)=10." 1>>steeringFile_${number}.txt
echo "CKIN(9)=-40." 1>>steeringFile_${number}.txt
echo "CKIN(10)=40." 1>>steeringFile_${number}.txt
echo "CKIN(11)=-40." 1>>steeringFile_${number}.txt
echo "CKIN(12)=40." 1>>steeringFile_${number}.txt
echo "CKIN(13)=-40." 1>>steeringFile_${number}.txt
echo "CKIN(14)=40." 1>>steeringFile_${number}.txt
echo "CKIN(15)=-40." 1>>steeringFile_${number}.txt
echo "CKIN(16)=40." 1>>steeringFile_${number}.txt
echo "CKIN(17)=-1." 1>>steeringFile_${number}.txt
echo "CKIN(18)=1." 1>>steeringFile_${number}.txt
echo "CKIN(19)=-1." 1>>steeringFile_${number}.txt
echo "CKIN(20)=1." 1>>steeringFile_${number}.txt
echo "CKIN(21)=0." 1>>steeringFile_${number}.txt
echo "CKIN(22)=1." 1>>steeringFile_${number}.txt
echo "CKIN(23)=0." 1>>steeringFile_${number}.txt
echo "CKIN(24)=1." 1>>steeringFile_${number}.txt
echo "CKIN(25)=-1." 1>>steeringFile_${number}.txt
echo "CKIN(26)=1." 1>>steeringFile_${number}.txt
echo "CKIN(27)=-1." 1>>steeringFile_${number}.txt
echo "CKIN(28)=1." 1>>steeringFile_${number}.txt
echo "CKIN(31)=2." 1>>steeringFile_${number}.txt
echo "CKIN(32)=-1." 1>>steeringFile_${number}.txt
echo "CKIN(35)=0." 1>>steeringFile_${number}.txt
echo "CKIN(36)=-1" 1>>steeringFile_${number}.txt
echo "CKIN(37)=0." 1>>steeringFile_${number}.txt
echo "CKIN(38)=-1." 1>>steeringFile_${number}.txt
echo "CKIN(39)=4." 1>>steeringFile_${number}.txt
echo "CKIN(40)=-1." 1>>steeringFile_${number}.txt
echo "CKIN(65)=1.e-09" 1>>steeringFile_${number}.txt        #! Min for Q^2
echo "CKIN(66)=-1." 1>>steeringFile_${number}.txt      #! Max for Q^2
echo "CKIN(67)=0." 1>>steeringFile_${number}.txt
echo "CKIN(68)=-1." 1>>steeringFile_${number}.txt
echo "CKIN(77)=2.0" 1>>steeringFile_${number}.txt
echo "CKIN(78)=-1." 1>>steeringFile_${number}.txt

# Run Pythia
pythiaeRHIC < steeringFile_${number}.txt > ${LOGDEST}/pythia_ep_noradcor_${ENRG}x${PNRG}_q2_${Q2MIN}_${Q2MAX}_run${number}.log

# Move Steering File
mv steeringFile_${number}.txt $STEERDEST/steeringFile_${ENRG}x${PNRG}_q2_${Q2MIN}_${Q2MAX}_run${number}.txt

# Move .txt File to Proper Location
mv pythia_ep_noradcor_${ENRG}x${PNRG}_q2_${Q2MIN}_${Q2MAX}_run${number}.txt $TXTDEST

# Create Tree
root -b -l -q /direct/eic+u/bpage/EPIC/officialSimu/pythia6/sidis_minbias/genTree.C\(\"${TXTDEST}/pythia_ep_noradcor_${ENRG}x${PNRG}_q2_${Q2MIN}_${Q2MAX}_run${number}.txt\",\"${TREEDEST}\"\)

# Create HEPMC
root -b -l -q /direct/eic+u/bpage/EPIC/officialSimu/pythia6/sidis_minbias/genHepMC.C\(\"${TREEDEST}/pythia_ep_noradcor_${ENRG}x${PNRG}_q2_${Q2MIN}_${Q2MAX}_run${number}.root\",\"${HEPMCDEST}\"\)

date




