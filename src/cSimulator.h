#include "cStation.h"
#include "cTrain.h"

namespace train
{
class cSimulator
{
public:

    int Time;
    std::vector< station_t > Stations;
    std::vector< train_t > Trains;

    cSimulator()
    : Time( 0 )
    {

    }

    /** Advance simulation one time step */
    void Advance();

    /** remove trains from simulation that have reached their destination */
    void RemoveTrainsAtDestination();
};
}
