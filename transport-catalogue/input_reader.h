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
        void AddToCatalogue(primary::Transport_catalogue& name, std::vector<std::string> data);
    }
}