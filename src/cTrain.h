#pragma once

namespace train {
class cTrain
{
public:

    cTrain( int track );

    void Advance();

    int LocationL()
    {
        return myLocationL;
    }
    int Track()
    {
        return myTrack;
    }

    std::string TextStatus();
    std::string TextPosition();
    std::string TextTrack();

    bool IsAtDestination() const
    {
        return myfDestination;
    }

private:
    int myTrack;                /// index of track running on
    int myLocationL;            /// distance in real world unit from start of track
    bool myfInStation;          /// true if in a station
    bool myfDestination;        /// true if in destination station
    int myTimeToDepart;         /// time to depart station
    int myIncrement;            /// pixel distance travelled in one time step
    station_t myStation;        /// station currently in
    station_t myPrevStation;    /// station most recently departed from
};

}

typedef std::shared_ptr< train::cTrain > train_t;



