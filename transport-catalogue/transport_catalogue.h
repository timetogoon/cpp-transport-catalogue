#pragma once

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <deque>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string_view>
#include <utility>
#include "geo.h"

namespace transport_catalogue 
{
    using namespace std::literals;

    struct Stop 
    {                       //остановка
        std::string name;
        coordinates::Coordinates coord;
    };

    struct Bus 
    {                        //маршрут
        std::string name;
        std::vector<Stop*> buses_stops;
        bool ring;
    };

    struct StopsForBusResponse //данные о маршруте
    {
        std::size_t stopsnumber;
        std::size_t uniquestops;
        double routelength;
        double curvature;
    };

    namespace detail 
    {
        class DistHasher //хеш-класс для мапы distances_
        {
        public:
            std::size_t operator()(const std::pair<const Stop*, const Stop*> pairptr) const
            {
                std::size_t ptr1hash = s_hasher_(pairptr.first);
                std::size_t ptr2hash = s_hasher_(pairptr.second);

                return ptr1hash * 37 + ptr2hash * 37 * 37;
            }
        private:
            std::hash<const Stop*> s_hasher_;
        };
    }

    namespace primary
    {
        class Transport_catalogue
        {
        public:
            void AddStop(std::string_view stop_name, const double latitude, const double longitude);

            void AddBus(const std::string& bus_number, const std::vector<const Stop*> stops, const bool ring);

            const Stop* GetStopPtr(const std::string& name) const;

            const Bus* GetBusptr(const std::string& name) const;

            const Bus FindBus(const std::string& name) const;

            const Stop FindStop(const std::string& name) const;

            const std::unordered_set<std::string_view> GetStopsForBuses(const std::string& name) const;

            const StopsForBusResponse GetBusInfo(const std::string& bus) const;

            void SetDistances(std::string_view stop1_name, std::string_view stop2_name, const std::size_t dist);

            std::size_t GetDistance(const std::string_view stop1_name, const std::string_view stop2_name) const;

        private:
            std::deque<Bus> buses_;
            std::unordered_map<std::string_view, Bus*> busname_to_bus_;
            std::deque<Stop> stops_;
            std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
            std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_to_buses_;
            std::unordered_map<std::pair<Stop*, Stop*>, std::size_t, detail::DistHasher> distances_;
        };
    }
}