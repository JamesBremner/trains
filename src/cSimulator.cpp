#include <string>
#include <fstream>
#include <iomanip>
#include <vector>
#include <memory>
#include <algorithm>

#include "nlohmann_json.hpp"

#include "cSimulator.h"

namespace train
{
void cSimulator::Advance()
{
    Time++;

    if( ! ( Time % 50 ) )
    {
        Trains.push_back( train_t( new cTrain( 0 )));
        Trains.push_back( train_t( new cTrain( 1 )));
    }

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

bool cSimulator::FileSave( const std::string& fpath )
{
    using json = nlohmann::json;

    std::ofstream ofs( fpath );
    if( !ofs )
        return false;

    json j =
    {
        {
            "title", myTitle
        },
    };

    json jstations;
    for( auto stat : Stations )
    {
        json js =
        {
            {
                "name",stat->myName
            },
            {
                "location", stat->myLocation
            }
        };
        jstations.push_back( js );
    }
    j["stations"] = jstations;

    ofs << std::setw(4) << j;

    return true;
}

bool cSimulator::FileOpen( const std::string& fpath )
{
    using json = nlohmann::json;

    std::ifstream ifs( fpath );
    if( ! ifs )
        return false;

    json j;
    j << ifs;

    myTitle = j["title"].get<std::string>();

    Stations.clear();
    Trains.clear();

    json js = j["stations"];
    for( json::iterator it = js.begin(); it < js.end(); it++ )
    {
        Stations.push_back( station_t( new cStation(
                                           (*it)["name"],
                                           (*it)["location"])));
    }

    return true;
}
}

