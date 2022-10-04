#pragma once

#include <vector>
#include <string>
#include "geo.h"

namespace domain
{
    struct Stop             //���������
    {                       
        std::string name;
        geo::Coordinates coord;
    };

    struct Bus              //�������
    {                        
        std::string name;
        std::vector<Stop*> buses_stops;
        bool ring;
    };

    struct StopsForBusResponse          //������ � ��������
    {
        std::size_t stopsnumber;
        std::size_t uniquestops;
        double routelength;
        double curvature;
    };

    namespace detail
    {
        class DistHasher            //���-����� ��� ���� distances_
        {
        public:
            std::size_t operator()(const std::pair<const Stop*, const Stop*> pairptr) const;
        private:
            std::hash<const Stop*> s_hasher_;
        };
    }
}