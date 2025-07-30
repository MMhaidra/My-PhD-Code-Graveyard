#!/bin/bash
EXTERDIR="/home/mmhaidra/ChanceSoft/cresta/external/"
cd $EXTERDIR
source $EXTERDIR/setupcresta.sh
cd ..
SCRIPTDIR="/home/mmhaidra/ChanceSoft/cresta/build/"
cd $SCRIPTDIR

#SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Get Inputs
echo "Getting Inputs"
JOB="lowZ"
RUNID=$PBS_ARRAYID
if [[ "$2" != "" ]]
then
   export USER=$2
fi
# make new temp scratch area
#echo "Creating Temp Scratch Area"
#scratchdir=$(source $SCRIPTDIR/make_temp_scratch.sh)
#cd $scratchdir
# GET OUTDIR and GEOMETRY

echo "Getting directories"
WORKDIR=$SCRIPTDIR
OUTDIR=$WORKDIR/mcfiles/
GEOMETRY=/home/mmhaidra/ChanceSoft/cresta/build/geofiles_largedrum_hydrobubble/largedrum_bub_2l_2l_4cm.geo

# Setup the framework
echo "Setting up code"
source $WORKDIR/setup.sh

# -------------------------------------------------------------------
# Run Job (2.5 hour exposure)
echo "Running"
chance_g4sim -g $GEOMETRY -t 9000 -o largedrum_bub_2l_2l_4cm_${JOB}  --run ${RUNID}
for file in ./largedrum_bub_2l_2l_4cm_${JOB}.${RUNID}.*.root;
do
chance_trackfit -i $file 
done
# Copy Outputs back to WORK Area
#cp -rv ./*.root $OUTDIR/
#rm chance_0_${JOB}.${RUNID}.*.root
#rm chance_0_${JOB}.${RUNID}.*.root.trackfit.root
# Remove temp directory
#rm -rv $scratchdir
