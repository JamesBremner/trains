#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include "cSimulator.h"

namespace train
{
void cSimulator::Advance()
{
    for( auto train : Trains )
    {
        train->Advance();
    }
    RemoveTrainsAtDestination();
}

void cSimulator::RemoveTrainsAtDestination()
{
    Trains.erase(
        std::remove_if( Trains.begin(), Trains.end(),
                        []( const train_t& train )
    {
        return train->IsAtDestination();
    }
                      ),
    Trains.end());
}
}

