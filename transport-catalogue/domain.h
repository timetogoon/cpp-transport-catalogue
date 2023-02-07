#pragma once

#include <vector>
#include <string>
#include "geo.h"

namespace domain
{
    struct Stop             // остановка
    {                       
        std::string name;
        geo::Coordinates coord;
    };

    struct Bus              // маршрут
    {                        
        std::string name;
        std::vector<Stop*> buses_stops;
        bool ring;
    };

    struct StopsForBusResponse          // данные о маршруте
    {
        std::size_t stopsnumber;
        std::size_t uniquestops;
        double routelength;
        double curvature;
    };

    namespace detail
    {
        class DistHasher            // хеш-класс для мапы distances_
        {
        public:
            std::size_t operator()(const std::pair<const Stop*, const Stop*> pairptr) const;
        private:
            std::hash<const Stop*> s_hasher_;
        };
    }

    struct Info                    // структура выходной информации о найденном маршруте (от остновки до остановки)
    {
        struct Wait                // остановка - ожидание
        {
            double minutes = 0;
            std::string stop_name;
        };

        struct Bus                 // информация о маршруте (название, кол-во пересадок, общее время поездки на маршруте)
        {
            std::string name;
            size_t span_count = 0;
            double minutes = 0.0;
        };

        Wait wait;
        Bus bus;
    };

    struct ReportRouter            // структура выходной информации (все сразу + общее время всех поездок)
    {
        std::vector<Info>information;
        double total_minutes = 0;
    };
}