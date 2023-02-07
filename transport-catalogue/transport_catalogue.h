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
#include "domain.h"

using namespace std::literals;
using namespace domain;

namespace transport_catalogue
{
    class Transport_catalogue
    {
    public:
        void AddStop(std::string_view stop_name, const double latitude, const double longitude);

        void AddBus(const std::string& bus_number, const std::vector<const Stop*> stops, const bool ring);

        const Stop* GetStopptr(const std::string& name) const;

        const Bus* GetBusptr(const std::string& name) const;

        const Bus FindBus(const std::string& name) const;

        const Stop FindStop(const std::string& name) const;

        const std::unordered_set<std::string_view>GetStopsForBuses(const std::string& name) const;

        const StopsForBusResponse GetBusInfo(const std::string& bus) const;

        const std::deque<Bus>GetBuses() const;

        const std::deque<Stop>GetStops() const;

        const std::unordered_map<std::string_view, Stop*>GetStopsPtr() const;

        const std::unordered_map<std::string_view, Bus*>GetBusesPtr() const;

        void SetDistances(std::string_view stop1_name, std::string_view stop2_name, const std::size_t dist);

        std::size_t GetDistance(const std::string_view stop1_name, const std::string_view stop2_name) const;

        std::unordered_map<std::pair<Stop*, Stop*>, std::size_t, domain::detail::DistHasher>GetDistancesAll() const;

    private:
        std::deque<Bus> buses_; // маршруты
        std::unordered_map<std::string_view, Bus*> busname_to_bus_;
        std::deque<Stop> stops_; // остановки
        std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
        std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_to_buses_;
        std::unordered_map<std::pair<Stop*, Stop*>, std::size_t, domain::detail::DistHasher> distances_;
    };
}
    
