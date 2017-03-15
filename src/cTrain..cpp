#include <cstdlib>
#include <string>
#include <vector>
#include <memory>

#include "cStation.h"
#include "cTrain.h"
#include "cSimulator.h"

namespace train {

cTrain::cTrain( int track )
    : myTrack( track )
    , myfInStation( true )
    , myfDestination( false )
    , myTimeToDepart( theSim.Time )
{
    if( myTrack == 0 )
    {
        myIncrement = 5;
        myLocationL = 0;
        myStation = *theSim.Stations.begin();
    }
    else
    {
        myLocationL = theSim.Stations.back()->myLocation - 2;
        myStation = theSim.Stations.back();
        myIncrement = -5;
    }
}

void cTrain::Advance()
{
    if( myfInStation )
    {
        if( theSim.Time < myTimeToDepart )
            return;

        // train ready to leave station
        // check the light
        if( ! myStation->IsLightGreen( myTrack ) )
            return;

        // Off we go!
        if( myPrevStation )
            myPrevStation->SetLight( myTrack, true );
        myPrevStation = myStation;
        myStation->SetLight( myTrack, false );
        myfInStation = false;
        if( myTrack == 0 )
            myLocationL += 6;
        else
            myLocationL -= 6;
    }

    myLocationL += myIncrement;


    for( auto station : theSim.Stations )
    {
        if( abs( myLocationL - station->myLocation) < 5 )
        {
            // train has arrived in station

            myLocationL  = station->myLocation;
            myfInStation = true;
            myTimeToDepart = theSim.Time + rand() % 10 + 1;
            myStation = station;

            // check if train is at final station
            if(
                ( myTrack == 0 && myLocationL == theSim.Stations.back()->myLocation ) ||
                ( myTrack == 1 && myLocationL == theSim.Stations[0]->myLocation ) )
            {
                myfDestination = true;
                myPrevStation->SetLight( myTrack, true );
            }
            break;
        }
    }
}
std::string cTrain::TextStatus()
{
    if( myfInStation )
        return "at";
    else
        return "leaving";
}
std::string cTrain::TextPosition()
{
    if( myPrevStation )
        return( myPrevStation->myName );
    else
        return "";
}
std::string cTrain::TextTrack()
{
    if( myTrack == 0 )
        return "Alpha";
    else
        return "Beta";
}
}


