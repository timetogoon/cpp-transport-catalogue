#pragma once

#include <vector>
#include <string>
#include "geo.h"

namespace domain
{
    struct Stop             //остановка
    {                       
        std::string name;
        geo::Coordinates coord;
    };

    struct Bus              //маршрут
    {                        
        std::string name;
        std::vector<Stop*> buses_stops;
        bool ring;
    };

    struct StopsForBusResponse          //данные о маршруте
    {
        std::size_t stopsnumber;
        std::size_t uniquestops;
        double routelength;
        double curvature;
    };

    namespace detail
    {
        class DistHasher            //хеш-класс для мапы distances_
        {
        public:
            std::size_t operator()(const std::pair<const Stop*, const Stop*> pairptr) const;
        private:
            std::hash<const Stop*> s_hasher_;
        };
    }
}