#pragma once

#include "transport_catalogue.h"
using namespace transport_catalogue;

namespace input 
{
    namespace detail {
        std::vector<std::string> ParseRequests(std::istream& input = std::cin);

        void InsertDataDist(primary::Transport_catalogue& name, std::vector<std::string> dist);
    }

    namespace parsing {

        std::string ParseStop(primary::Transport_catalogue& name, std::string data);

        void ParseBus(primary::Transport_catalogue& name, std::string data);

        void AddToCatalogue(primary::Transport_catalogue& name, std::vector<std::string> data);
    }
}