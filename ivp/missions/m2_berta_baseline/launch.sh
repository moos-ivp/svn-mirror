#!/bin/bash -e
#---------------------------------------------------------------
#   Script: launch.sh
#  Mission: m2_berta
#   Author: Michael Benjamin
#   LastEd: May 8th 2021
#---------------------------------------------------------------
#  Part 1: Set global var defaults
#---------------------------------------------------------------
ME=`basename "$0"`
TIME_WARP=1
JUST_MAKE=""
VERBOSE=""
AUTO_LAUNCHED="no"

VNAME1="henry"    
VNAME2="gilda"    

#---------------------------------------------------------------
#  Part 2: Check for and handle command-line arguments
#---------------------------------------------------------------
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ]; then
	echo "$ME [SWITCHES] [time_warp]                                   " 
	echo "  --help, -h           Show this help message                " 
	echo "  --just_make, -j      Just make targ files, no launch       " 
	echo "  --verbose, -v        Verbose output, confirm before launch " 
	exit 0;
    elif [ "${ARGI//[^0-9]/}" = "$ARGI" -a "$TIME_WARP" = 1 ]; then 
        TIME_WARP=$ARGI
    elif [ "${ARGI}" = "--just_make" -o "${ARGI}" = "-j" ]; then
	JUST_MAKE="-j"
    elif [ "${ARGI}" = "--verbose" -o "${ARGI}" = "-v" ]; then
	VERBOSE="--verbose"
    else 
        echo "$ME: Bad arg:" $ARGI "Exit Code 1."
        exit 1
    fi
done

#---------------------------------------------------------------
#  Part 3: Launch the vehicles
#---------------------------------------------------------------
VLAUNCH_ARGS=" --auto --vname=henry --index=1 --start=170,-80 "
VLAUNCH_ARGS+="--hdg=270 --spd=0 --loiter_pos=x=0,y=-95 "
echo "$ME: Launching $VNAME1 ..."
./launch_vehicle.sh $VLAUNCH_ARGS $VERBOSE $JUST_MAKE $TIME_WARP

VLAUNCH_ARGS=" --auto --vname=gilda --index=2 --start=-30,-80 "
VLAUNCH_ARGS+="--hdg=90 --spd=0 --loiter_pos=x=125,y=-65 "
echo "$ME: Launching $VNAME2 ..."
./launch_vehicle.sh $VLAUNCH_ARGS $VERBOSE $JUST_MAKE $TIME_WARP


#---------------------------------------------------------------
#  Part 4: Launch the shoreside
#---------------------------------------------------------------
echo "Launching Shoreside ..."
SLAUNCH_ARGS=" --auto --vnames=$VNAME1:$VNAME2 "
./launch_shoreside.sh $SLAUNCH_ARGS $VERBOSE $JUST_MAKE $TIME_WARP
       SHORE_PSHARE=$SHORE_PSHARE  SHORE_MOOSDB="9000"        \
       VNAMES=$VNAME1:$VNAME2

#---------------------------------------------------------------                                         
#  Part 7: If launched from script, we're done, exit now                                                 
#---------------------------------------------------------------                                         
if [ "${AUTO_LAUNCHED}" = "yes" -o "${JUST_MAKE}" != "" ]; then
    exit 0
fi

#---------------------------------------------------------------                                         
# Part 8: Launch uMAC in paused mode until the mission is quit                                           
#---------------------------------------------------------------                                         
uMAC --paused targ_shoreside.moos
kill -- -$$
