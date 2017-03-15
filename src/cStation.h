#pragma once

namespace train {
    class cStation
{
public:
    std::string myName;
    int myLocation;


    cStation( const std::string& name, int loc )
        : myName( name )
        , myLocation( loc )
        , myLightGreen( true )
        , myTrackBetaLightGreen( true )
    {

    }

    bool IsLightGreen( int track )
    {
        if( track == 0 )
            return myLightGreen;
        else
            return myTrackBetaLightGreen;
    }
    void SetLight( int track, bool green )
    {
        if( track == 0 )
            myLightGreen = green;
        else
            myTrackBetaLightGreen = green;
    }

private:
    bool myLightGreen;
    bool myTrackBetaLightGreen;
};
}

typedef std::shared_ptr< train::cStation > station_t;

