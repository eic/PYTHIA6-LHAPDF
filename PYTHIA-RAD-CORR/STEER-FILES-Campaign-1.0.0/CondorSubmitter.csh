#! /bin/env csh

set Exec = /direct/eic+u/bpage/EPIC/officialSimu/pythia6/sidis_minbias/epCreateSimuCondor.sh

# make sure executable exists
#make $Exec || exit

####### Initialize condor file
echo ""  > CondorFile
echo "Universe     = vanilla" >> CondorFile
echo "Executable   = ${Exec}" >> CondorFile
echo "getenv = true" >> CondorFile

# Output Directory
set Output = /gpfs/mnt/gpfs02/eic/bpage/home/EPIC/officialSimu/pythia6/sidis_minbias/condorLogs

set NUMBER = 1
set LIMIT = 50
set Q2LO = 0.000000001
set Q2HI = 1.0
set ENRG = 5
set PNRG = 41

while ( "$NUMBER" <= "$LIMIT" )

    set LogFile = ${Output}/pythia_ep_noradcor_${ENRG}x${PNRG}_q2_${Q2LO}_${Q2HI}_run${NUMBER}.out
    set ErrFile = ${Output}/pythia_ep_noradcor_${ENRG}x${PNRG}_q2_${Q2LO}_${Q2HI}_run${NUMBER}.err
    
    set Args = ( $NUMBER $Q2LO $Q2HI $ENRG $PNRG )
    echo "" >> CondorFile
    echo "Output       = ${LogFile}" >> CondorFile
    echo "Error        = ${ErrFile}" >> CondorFile
    echo "Arguments    = ${Args}" >> CondorFile
    echo "Queue" >> CondorFile   

    echo Submitting:
    echo $Exec $Args
    echo "Logging output to " $LogFile
    echo "Logging errors to " $ErrFile

    @ NUMBER++

end


#foreach input ( ${base}* )
#    # arguments
#    set OutBase=`basename $input | sed 's/.root//g'`
#    set OutName    = ${Output}/out_${OutBase}.root
#    set Files      = ${input}
#    
#    # Logfiles.
#    set LogFile    = ${Output}/logs/out_${OutBase}.out
#    set ErrFile    = ${Output}/logs/out_${OutBase}.err
#
#    ### hand to condor
#    #set Args = ( -o $OutName -i $Files -breit $breit )
#    set Args = ( 0 $Files $OutName )
#    echo "" >> CondorFile
#    echo "Output       = ${LogFile}" >> CondorFile
#    echo "Error        = ${ErrFile}" >> CondorFile
#    echo "Arguments    = ${Args}" >> CondorFile
#    echo "Queue" >> CondorFile   
#
#    echo Submitting:
#    echo $Exec $Args
#    echo "Logging output to " $LogFile
#    echo "Logging errors to " $ErrFile
#    echo
#end
#condor_submit CondorFile


