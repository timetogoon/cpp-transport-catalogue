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
    
    void InsertDataDist(primary::Transport_catalogue& name, vector<string> dist)
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
    string ParseStop(primary::Transport_catalogue& name, string data)
    {
        string todist;
        data = data.substr(5, data.npos);
        auto bus_stop_name = data.substr(data.find_first_not_of(' '), data.find(':'));
        data = data.substr(data.find_first_of(':') + 2, data.find_last_not_of(' '));
        double lat = 0.0;
        double longt = 0.0;
        stringstream sslat, sslong;
        sslat << data.substr(data.find_first_not_of(' '), data.find_first_of(','));
        sslat >> lat;
        data.erase(data.find_first_not_of(' '), data.find_first_of(',') + 2);
        if (data.find_first_of(',') != data.npos)
        {
            sslong << data.substr(data.find_first_not_of(' '), data.find_first_of(','));
            data.erase(data.find_first_not_of(' '), data.find_first_of(',') + 2);
            todist = bus_stop_name + ',' + ' ' + data;
        }
        else
        {
            sslong << data.substr(data.find_first_not_of(' '), data.find_last_not_of(' ') + 1);
        }
        sslong >> longt;
        name.AddStop(bus_stop_name, lat, longt);
        return todist;
    }

    void ParseBus(primary::Transport_catalogue& name, string data)
    {
        auto bus_number = data.substr(4, data.find_first_of(":") - 4);
        vector<const Stop*> bus_stops;
        if (data.find('>') != data.npos) //если маршрут кольцевой
        {
            data = data.substr(data.find(':') + 2, data.find_last_not_of(' '));
            while (data.length() != 0) {
                if (data.find('>') == data.npos)
                {
                    bus_stops.push_back(name.GetStopPtr(data.substr(data.find_first_not_of(' '),
                        data.find_last_not_of(' ') + 1)));
                    data.clear();
                }
                else
                {
                    bus_stops.push_back(name.GetStopPtr(data.substr(data.find_first_not_of(' '),
                        data.find_first_of('>') - 1)));
                    data = data.substr(data.find_first_of('>') + 2, data.find_last_not_of(' '));
                }
            }
            name.AddBus(bus_number, bus_stops, true);
        }
        else
        {
            data = data.substr(data.find(':') + 2, data.find_last_not_of(' '));
            while (data.length() != 0) {
                if (data.find('-') == data.npos)
                {
                    bus_stops.push_back(name.GetStopPtr(data.substr(data.find_first_not_of(' '),
                        data.find_last_not_of(' ') + 1)));
                    data.clear();
                }
                else
                {
                    bus_stops.push_back(name.GetStopPtr(data.substr(data.find_first_not_of(' '),
                        data.find_first_of('-') - 1)));
                    data = data.substr(data.find_first_of('-') + 2,
                        data.find_last_not_of(' '));
                }
            }
            name.AddBus(bus_number, bus_stops, false);
        }
    }

    void AddToCatalogue(primary::Transport_catalogue& name, vector<string> data)
    {
        using namespace literals;

        vector<string> todist;

        for (int i = 0; i < data.size(); i++)
        {
            auto command = data[i].substr(data[i].find_first_not_of(' '), 4);
            if (command.find("Stop"s) != data[i].npos)  //вставляем остановку
            {
                auto temp = ParseStop(name, data[i]);
                if (temp.empty())
                {
                    continue;
                }
                else
                {
                    todist.push_back(temp);
                } 
            }
            else if (command.find("Bus"s) != data[i].npos) //вставляем маршрут
            {
                ParseBus(name, data[i]);
            }
        }
        input::detail::InsertDataDist(name, move(todist));
    }
}