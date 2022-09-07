#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue::primary
{
    void Transport_catalogue::AddStop(string_view stop_name, const double latitude, const double longitude)
    {
        stops_.push_back({ string(stop_name), { latitude, longitude } });
        stopname_to_stop_.emplace(stops_.back().name, &stops_.back());
        stop_to_buses_[stops_.back().name];
    }

    void Transport_catalogue::AddBus(const string& bus_number, const vector<const Stop*> stops, const bool ring)
    {
        Bus bus;
        bus.name = bus_number;
        for (auto& stop : stops)
        {
            bus.buses_stops.push_back(const_cast<Stop*>(stop));
        }
        bus.ring = ring;
        buses_.push_back(bus);
        busname_to_bus_.emplace(buses_.back().name, &buses_.back());
        for (auto& stop : stops)
        {
            stop_to_buses_[GetStopptr(stop->name)->name].insert(GetBusptr(bus_number)->name);
        }
        
    }

    const Stop* Transport_catalogue::GetStopptr(const string& name) const 
    {
        if (stopname_to_stop_.count(name) == 0)
        {
            throw invalid_argument("stop does not exists!");
        }
        else
        {
            return stopname_to_stop_.at(name);
        }
    }

    const Bus* Transport_catalogue::GetBusptr(const string& name) const
    {
        if (busname_to_bus_.count(name) == 0)
        {
            throw invalid_argument("Bus does not exists!");
        }
        else
        {
            return busname_to_bus_.at(name);
        }
    }

    const Bus Transport_catalogue::FindBus(const string& name) const
    {
        if (busname_to_bus_.count(name) == 0)
        {
            return {};
        }
        else
        {
            return *busname_to_bus_.at(name);
        }
    }

    const Stop Transport_catalogue::FindStop(const std::string& name) const
    {
        if (stopname_to_stop_.count(name) == 0)
        {
            return {};
        }
        else
        {
            return *stopname_to_stop_.at(name);
        }
    }

    const std::unordered_set<std::string_view> Transport_catalogue::GetStopsForBuses(const std::string& name) const
    {
        return stop_to_buses_.at(name);
    }

    const StopsForBusResponse Transport_catalogue::GetBusInfo(const string& bus) const 
    {
        StopsForBusResponse businfo = {};
        if (busname_to_bus_.count(bus) == 0)
        {
            return businfo;
        }
        else {
            double length = 0.0, dist = 0.0;
            size_t stopsquantity = 0;
            for (auto i = 0; i < busname_to_bus_.at(bus)->buses_stops.size() - 1; i++)
            {                
                length += coordinates::ComputeDistance(busname_to_bus_.at(bus)->buses_stops[i]->coord,
                    busname_to_bus_.at(bus)->buses_stops[i + 1]->coord);
                if (distances_.count({ busname_to_bus_.at(bus)->buses_stops[i], //если есть *A,*B
                    busname_to_bus_.at(bus)->buses_stops[i + 1] }) > 0) 
                {
                    dist += distances_.at({ busname_to_bus_.at(bus)->buses_stops[i],
                                        busname_to_bus_.at(bus)->buses_stops[i + 1] });
                }
                else
                {
                    dist += distances_.at({ busname_to_bus_.at(bus)->buses_stops[i + 1], //если нет *A,*B, то искать *B,*A
                                        busname_to_bus_.at(bus)->buses_stops[i] });
                }                
            }
            if (!busname_to_bus_.at(bus)->ring) //если маршрут не кольцевой
            {
                length *= 2; //увеличить географическую длину на 2
                stopsquantity = busname_to_bus_.at(bus)->buses_stops.size() * 2 - 1;
                for (auto i = 0; i < busname_to_bus_.at(bus)->buses_stops.size() - 1; i++)
                {
                    if (distances_.count({ busname_to_bus_.at(bus)->buses_stops[i + 1], //если есть *A,*B
                        busname_to_bus_.at(bus)->buses_stops[i] }) > 0)
                    {
                        dist += distances_.at({ busname_to_bus_.at(bus)->buses_stops[i + 1],
                                            busname_to_bus_.at(bus)->buses_stops[i] });
                    }
                    else
                    {
                        dist += distances_.at({ busname_to_bus_.at(bus)->buses_stops[i], //если нет *A,*B, то искать *B,*A
                                            busname_to_bus_.at(bus)->buses_stops[i + 1] });
                    }
                }
            }
            else
            {
                stopsquantity = busname_to_bus_.at(bus)->buses_stops.size();
            }
            unordered_set<string_view> uniq;
            for (auto st : busname_to_bus_.at(bus)->buses_stops) {
                uniq.emplace(st->name);
            }
            businfo = { stopsquantity, uniq.size(), dist, dist/length};
        }
        return businfo;
    }

    void Transport_catalogue::SetDistances(string_view stop1_name, string_view stop2_name, const size_t dist) 
    {
        auto ptr1 = GetStopptr(string(stop1_name));
        auto ptr2 = GetStopptr(string(stop2_name));
        distances_[{const_cast<Stop*>(ptr1), const_cast<Stop*>(ptr2)}] = dist;
    }

    size_t Transport_catalogue::GetDistance(const string_view stop1_name, const string_view stop2_name) const
    {
        auto ptr1 = GetStopptr(string(stop1_name));
        auto ptr2 = GetStopptr(string(stop2_name));
        if (distances_.count({ const_cast<Stop*>(ptr1), const_cast<Stop*>(ptr2) }) > 0)
        {
            return distances_.at({ const_cast<Stop*>(ptr1), const_cast<Stop*>(ptr2) });
        }
        else
        {
            return 0;
        }
    }
}
