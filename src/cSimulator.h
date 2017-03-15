#include "cStation.h"
#include "cTrain.h"

namespace train
{
class cSimulator
{
public:

    int Time;
    std::string myTitle;
    std::vector< station_t > Stations;
    std::vector< train_t > Trains;

    cSimulator()
    : Time( 0 )
    , myTitle("test")
    {

    }

    /** Advance simulation one time step */
    void Advance();

    bool FileSave( const std::string& fpath );
    bool FileOpen( const std::string& fpath );

    /** remove trains from simulation that have reached their destination */
    void RemoveTrainsAtDestination();
};
}

extern train::cSimulator theSim;
