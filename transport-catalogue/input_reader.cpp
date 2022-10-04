#include "input_reader.h"

using namespace std;

namespace input::detail 
{
    vector<string> ParseRequests(istream& input)
    {
        vector<string> result;
        size_t request = 0;
        (input >> request).get();
        for (int i = 0; i < request; i++)
        {
            string line;
            getline(cin, line, '\n');
            result.push_back(line);
        }
        sort(result.begin(), result.end());
        reverse(result.begin(), result.end());
        return result;
    }
    
    void InsertDataDist(Transport_catalogue& name, vector<string> dist)
    {
        for (int i = 0; i < dist.size(); i++)
        {
            auto stop_name1 = dist[i].substr(dist[i].find_first_not_of(' '), dist[i].find(','));
            dist[i].erase(dist[i].find_first_not_of(' '), dist[i].find(',') + 2);
            while (!dist[i].empty())
            {
                stringstream ss;
                size_t distance = 0;
                ss << dist[i].substr(dist[i].find_first_not_of(' '), dist[i].find_first_of('m'));
                ss >> distance;
                dist[i].erase(dist[i].find_first_not_of(' '), dist[i].find_first_of('m') + 2);
                auto stop_name2 = dist[i].substr(dist[i].find_first_of(' ') + 1, dist[i].find_first_of(',') - 3);
                if (dist[i].find(',') != dist[i].npos)
                {
                    dist[i].erase(dist[i].find_first_not_of(' '), dist[i].find_first_of(',') + 2);
                }
                else
                {
                    dist[i].erase(dist[i].find_first_not_of(' '), dist[i].npos);
                }
                name.SetDistances(stop_name1, stop_name2, distance);
            }
        }
    }
}

namespace input::parsing
{
void AddToCatalogue(Transport_catalogue& name, vector<string> data)
{
    using namespace literals;

    vector<string> todist;
    for (int i = 0; i < data.size(); i++)
    {
        auto command = data[i].substr(data[i].find_first_not_of(' '), 4);
        if (command.find("Stop"s) != data[i].npos)  //вставляем остановку
        {
            data[i] = data[i].substr(5, data[i].npos);
            auto bus_stop_name = data[i].substr(data[i].find_first_not_of(' '), data[i].find(':'));
            data[i] = data[i].substr(data[i].find_first_of(':') + 2, data[i].find_last_not_of(' '));
            double coordlat = 0.0, coordlongt = 0.0;
            stringstream sslat, sslong;
            sslat << data[i].substr(data[i].find_first_not_of(' '), data[i].find_first_of(','));
            sslat >> coordlat;
            data[i].erase(data[i].find_first_not_of(' '), data[i].find_first_of(',') + 2);
            if (data[i].find_first_of(',') != data[i].npos)
            {
                sslong << data[i].substr(data[i].find_first_not_of(' '), data[i].find_first_of(','));
                data[i].erase(data[i].find_first_not_of(' '), data[i].find_first_of(',') + 2);
                todist.push_back(bus_stop_name + ',' + ' ' + data[i]);
            }
            else
            {
                sslong << data[i].substr(data[i].find_first_not_of(' '), data[i].find_last_not_of(' ') + 1);
            }
            sslong >> coordlongt;
            name.AddStop(bus_stop_name, coordlat, coordlongt);
        }
        else if (command.find("Bus"s) != data[i].npos) //вставляем маршрут
        {
            auto bus_number = data[i].substr(4, data[i].find_first_of(":") - 4);
            vector<const Stop*> bus_stops;
            if (data[i].find('>') != data[i].npos) //если маршрут кольцевой
            {
                data[i] = data[i].substr(data[i].find(':') + 2, data[i].find_last_not_of(' '));
                while (data[i].length() != 0) {
                    if (data[i].find('>') == data[i].npos)
                    {
                        bus_stops.push_back(name.GetStopptr(data[i].substr(data[i].find_first_not_of(' '),
                            data[i].find_last_not_of(' ') + 1)));
                        data[i].clear();
                    }
                    else
                    {
                        bus_stops.push_back(name.GetStopptr(data[i].substr(data[i].find_first_not_of(' '),
                            data[i].find_first_of('>') - 1)));
                        data[i] = data[i].substr(data[i].find_first_of('>') + 2, data[i].find_last_not_of(' '));
                    }
                }
                name.AddBus(bus_number, bus_stops, true);
            }
            else
            {
                data[i] = data[i].substr(data[i].find(':') + 2, data[i].find_last_not_of(' '));
                while (data[i].length() != 0) {
                    if (data[i].find('-') == data[i].npos)
                    {
                        bus_stops.push_back(name.GetStopptr(data[i].substr(data[i].find_first_not_of(' '),
                            data[i].find_last_not_of(' ') + 1)));
                        data[i].clear();
                    }
                    else
                    {
                        bus_stops.push_back(name.GetStopptr(data[i].substr(data[i].find_first_not_of(' '),
                            data[i].find_first_of('-') - 1)));
                        data[i] = data[i].substr(data[i].find_first_of('-') + 2,
                            data[i].find_last_not_of(' '));
                    }
                }
                name.AddBus(bus_number, bus_stops, false);
            }
        }
    }
    input::detail::InsertDataDist(name, move(todist));
}
}